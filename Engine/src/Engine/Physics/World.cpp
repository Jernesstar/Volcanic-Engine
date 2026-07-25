#include "World.h"

#include <mutex>
#include <unordered_map>
#include <vector>

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>

#include <VolcaniCore/Core/Log.h>

#include "Physics.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Physics {

const float PhysicsWorld::StepSize = 1.0f / 60.0f;

// ---------------------------------------------------------------------------
// Layer configuration
// ---------------------------------------------------------------------------
namespace BroadPhaseLayers {
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr JPH::BroadPhaseLayer SENSOR(2);
	static constexpr JPH::uint NUM_LAYERS(3);
}

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
public:
	bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override {
		// Statics never collide with statics. Sensors overlap everything so
		// they can detect any body entering them.
		if(a == Layers::SENSOR || b == Layers::SENSOR)
			return true;
		if(a == Layers::NON_MOVING && b == Layers::NON_MOVING)
			return false;
		return true;
	}
};

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
	JPH::uint GetNumBroadPhaseLayers() const override {
		return BroadPhaseLayers::NUM_LAYERS;
	}

	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
		switch(layer) {
			case Layers::NON_MOVING: return BroadPhaseLayers::NON_MOVING;
			case Layers::MOVING:	 return BroadPhaseLayers::MOVING;
			case Layers::SENSOR:	 return BroadPhaseLayers::SENSOR;
		}
		return BroadPhaseLayers::NON_MOVING;
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer) const override {
		return "Layer";
	}
#endif
};

class ObjectVsBroadPhaseLayerFilterImpl final
	: public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer a, JPH::BroadPhaseLayer b) const override {
		if(a == Layers::SENSOR)
			return true;
		if(a == Layers::NON_MOVING)
			return b != BroadPhaseLayers::NON_MOVING || true; // statics can be hit
		return true;
	}
};

// ---------------------------------------------------------------------------
// Contact listener: bridges sensor overlaps to TriggerEvents
// ---------------------------------------------------------------------------
struct PhysicsWorld::Impl : public JPH::ContactListener {
	JPH::TempAllocatorImpl* TempAllocator = nullptr;
	JPH::JobSystemThreadPool* JobSystem = nullptr;
	JPH::PhysicsSystem System;

	BPLayerInterfaceImpl BroadPhaseLayerInterface;
	ObjectVsBroadPhaseLayerFilterImpl ObjectVsBroadPhaseLayerFilter;
	ObjectLayerPairFilterImpl ObjectLayerPairFilter;

	float Accumulator = 0.0f;

	// entity id <-> body user-data resolution is direct: body user data holds
	// the entity id. We also track sensor bodies to know which side of an
	// overlap is the trigger.
	std::unordered_map<uint32_t, bool> IsSensorBody; // BodyID index -> sensor?

	std::mutex CallbackMutex;
	std::vector<std::pair<uint32_t, TriggerCallback>> Callbacks;
	uint32_t NextToken = 1;

	// Pending events collected during a step (contact callbacks run on job
	// threads); dispatched on the calling thread after the step completes.
	std::mutex EventMutex;
	std::vector<TriggerEvent> PendingEvents;

	uint64_t EntityOf(const JPH::Body& body) const {
		return (uint64_t)body.GetUserData();
	}

	void Dispatch() {
		std::vector<TriggerEvent> events;
		{
			std::lock_guard<std::mutex> lock(EventMutex);
			events.swap(PendingEvents);
		}
		if(events.empty())
			return;

		std::vector<std::pair<uint32_t, TriggerCallback>> callbacks;
		{
			std::lock_guard<std::mutex> lock(CallbackMutex);
			callbacks = Callbacks;
		}
		for(const auto& ev : events)
			for(const auto& [token, cb] : callbacks)
				if(cb)
					cb(ev);
	}

	void EmitPair(const JPH::Body& b1, const JPH::Body& b2, bool enter) {
		bool s1 = b1.IsSensor();
		bool s2 = b2.IsSensor();
		if(!s1 && !s2)
			return;

		std::lock_guard<std::mutex> lock(EventMutex);
		if(s1) {
			PendingEvents.push_back(
				TriggerEvent{ EntityOf(b1), EntityOf(b2), enter });
		}
		if(s2) {
			PendingEvents.push_back(
				TriggerEvent{ EntityOf(b2), EntityOf(b1), enter });
		}
	}

	// JPH::ContactListener
	void OnContactAdded(const JPH::Body& b1, const JPH::Body& b2,
		const JPH::ContactManifold&, JPH::ContactSettings&) override
	{
		EmitPair(b1, b2, true);
	}

	void OnContactRemoved(const JPH::SubShapeIDPair& pair) override {
		// We only have body ids here; look them up through the body interface.
		auto& bi = System.GetBodyInterfaceNoLock();
		// Bodies may already be removed; guard by checking added state.
		uint64_t e1 = (uint64_t)bi.GetUserData(pair.GetBody1ID());
		uint64_t e2 = (uint64_t)bi.GetUserData(pair.GetBody2ID());

		bool s1 = IsSensorBody.count(pair.GetBody1ID().GetIndex())
			&& IsSensorBody[pair.GetBody1ID().GetIndex()];
		bool s2 = IsSensorBody.count(pair.GetBody2ID().GetIndex())
			&& IsSensorBody[pair.GetBody2ID().GetIndex()];

		if(!s1 && !s2)
			return;

		std::lock_guard<std::mutex> lock(EventMutex);
		if(s1)
			PendingEvents.push_back(TriggerEvent{ e1, e2, false });
		if(s2)
			PendingEvents.push_back(TriggerEvent{ e2, e1, false });
	}
};

// ---------------------------------------------------------------------------
// PhysicsWorld
// ---------------------------------------------------------------------------
PhysicsWorld::PhysicsWorld() {
	// Jolt's global allocator/type factory must be live before any Jolt object
	// (including the temp allocator below) is constructed. Init is refcounted
	// and safe to call once per world.
	Physics::Init();

	m_Impl = std::make_unique<Impl>();

	const JPH::uint cMaxBodies = 10240;
	const JPH::uint cNumBodyMutexes = 0;
	const JPH::uint cMaxBodyPairs = 65536;
	const JPH::uint cMaxContactConstraints = 20480;

	m_Impl->TempAllocator = new JPH::TempAllocatorImpl(16 * 1024 * 1024);
	m_Impl->JobSystem = new JPH::JobSystemThreadPool(
		JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
		(int)std::max(1u, std::thread::hardware_concurrency() - 1));

	m_Impl->System.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
		cMaxContactConstraints, m_Impl->BroadPhaseLayerInterface,
		m_Impl->ObjectVsBroadPhaseLayerFilter, m_Impl->ObjectLayerPairFilter);

	m_Impl->System.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
	m_Impl->System.SetContactListener(m_Impl.get());
}

PhysicsWorld::~PhysicsWorld() {
	if(m_Impl) {
		delete m_Impl->JobSystem;
		delete m_Impl->TempAllocator;
		m_Impl.reset();
	}
	Physics::Close();
}

void PhysicsWorld::OptimizeBroadPhase() {
	m_Impl->System.OptimizeBroadPhase();
}

void PhysicsWorld::Step(float dt) {
	if(dt <= 0.0f)
		return;

	m_Impl->Accumulator += dt;

	// Clamp runaway accumulation (e.g. after a long editor pause) so we don't
	// spiral into a huge catch-up loop.
	const float maxAccum = StepSize * 8.0f;
	if(m_Impl->Accumulator > maxAccum)
		m_Impl->Accumulator = maxAccum;

	while(m_Impl->Accumulator >= StepSize) {
		m_Impl->Accumulator -= StepSize;
		m_Impl->System.Update(
			StepSize, 1, m_Impl->TempAllocator, m_Impl->JobSystem);
	}

	m_Impl->Dispatch();
}

void PhysicsWorld::SetGravity(const Vec3& g) {
	m_Impl->System.SetGravity(JPH::Vec3(g.x, g.y, g.z));
}

Vec3 PhysicsWorld::GetGravity() const {
	auto g = m_Impl->System.GetGravity();
	return Vec3(g.GetX(), g.GetY(), g.GetZ());
}

static JPH::Quat EulerToQuat(const Vec3& euler) {
	return JPH::Quat::sEulerAngles(JPH::Vec3(euler.x, euler.y, euler.z));
}

BodyHandle PhysicsWorld::AddStaticBody(const Ref<Shape>& shape,
	const Transform& transform, uint64_t entity)
{
	if(!shape || !shape->IsValid())
		return BodyHandle{};

	auto& bi = m_Impl->System.GetBodyInterface();

	JPH::BodyCreationSettings settings(
		shape->Get(),
		JPH::RVec3(transform.Translation.x, transform.Translation.y,
			transform.Translation.z),
		EulerToQuat(transform.Rotation),
		JPH::EMotionType::Static,
		Layers::NON_MOVING);
	settings.mUserData = (JPH::uint64)entity;

	JPH::BodyID id = bi.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
	m_Impl->IsSensorBody[id.GetIndex()] = false;

	return BodyHandle{ id.GetIndexAndSequenceNumber() };
}

BodyHandle PhysicsWorld::AddSensorBody(const Ref<Shape>& shape,
	const Transform& transform, uint64_t entity)
{
	if(!shape || !shape->IsValid())
		return BodyHandle{};

	auto& bi = m_Impl->System.GetBodyInterface();

	JPH::BodyCreationSettings settings(
		shape->Get(),
		JPH::RVec3(transform.Translation.x, transform.Translation.y,
			transform.Translation.z),
		EulerToQuat(transform.Rotation),
		JPH::EMotionType::Static,
		Layers::SENSOR);
	settings.mIsSensor = true;
	settings.mUserData = (JPH::uint64)entity;

	// Sensors need collision detection enabled against everything; keep them
	// non-moving statics so they don't fall.
	JPH::BodyID id = bi.CreateAndAddBody(settings, JPH::EActivation::Activate);
	m_Impl->IsSensorBody[id.GetIndex()] = true;

	return BodyHandle{ id.GetIndexAndSequenceNumber() };
}

void PhysicsWorld::RemoveBody(BodyHandle handle) {
	if(!handle.IsValid())
		return;
	JPH::BodyID id(handle.ID);
	auto& bi = m_Impl->System.GetBodyInterface();
	m_Impl->IsSensorBody.erase(id.GetIndex());
	bi.RemoveBody(id);
	bi.DestroyBody(id);
}

void PhysicsWorld::SetBodyTransform(BodyHandle handle, const Transform& t) {
	if(!handle.IsValid())
		return;
	JPH::BodyID id(handle.ID);
	auto& bi = m_Impl->System.GetBodyInterface();
	bi.SetPositionAndRotation(id,
		JPH::RVec3(t.Translation.x, t.Translation.y, t.Translation.z),
		EulerToQuat(t.Rotation),
		JPH::EActivation::DontActivate);
}

Transform PhysicsWorld::GetBodyTransform(BodyHandle handle) const {
	Transform t;
	if(!handle.IsValid())
		return t;

	JPH::BodyID id(handle.ID);
	auto& bi = m_Impl->System.GetBodyInterface();
	JPH::RVec3 pos = bi.GetPosition(id);
	t.Translation = Vec3(pos.GetX(), pos.GetY(), pos.GetZ());

	JPH::Vec3 euler = bi.GetRotation(id).GetEulerAngles();
	t.Rotation = Vec3(euler.GetX(), euler.GetY(), euler.GetZ());
	return t;
}

// Broadphase/object filters that respect a caller-supplied layer bitmask.
namespace {

class MaskBroadPhaseFilter : public JPH::BroadPhaseLayerFilter {
public:
	explicit MaskBroadPhaseFilter(uint32_t mask) : m_Mask(mask) { }
	bool ShouldCollide(JPH::BroadPhaseLayer) const override { return true; }
private:
	uint32_t m_Mask;
};

class MaskObjectLayerFilter : public JPH::ObjectLayerFilter {
public:
	explicit MaskObjectLayerFilter(uint32_t mask) : m_Mask(mask) { }
	bool ShouldCollide(JPH::ObjectLayer layer) const override {
		// ObjectLayer values are small indices (Layers::*). Guard the shift.
		if(layer >= 32)
			return true;
		return (m_Mask & (1u << layer)) != 0;
	}
private:
	uint32_t m_Mask;
};

}

RayHit PhysicsWorld::Raycast(const Vec3& origin, const Vec3& direction,
	float maxDist, uint32_t layerMask) const
{
	RayHit hit;

	Vec3 dir = direction;
	float len = glm::length(dir);
	if(len < 1e-8f)
		return hit;
	dir /= len;

	// By default exclude sensors from raycasts (they are overlap-only). Callers
	// who explicitly set the sensor bit opt back in.
	if(layerMask == 0xFFFFFFFF)
		layerMask &= ~(1u << Layers::SENSOR);

	JPH::RRayCast ray{
		JPH::RVec3(origin.x, origin.y, origin.z),
		JPH::Vec3(dir.x, dir.y, dir.z) * maxDist
	};

	JPH::RayCastResult result;
	MaskBroadPhaseFilter bpFilter(layerMask);
	MaskObjectLayerFilter objFilter(layerMask);

	const auto& query = m_Impl->System.GetNarrowPhaseQuery();
	bool hasHit = query.CastRay(ray, result, bpFilter, objFilter);
	if(!hasHit)
		return hit;

	auto& bi = m_Impl->System.GetBodyInterfaceNoLock();
	hit.HasHit = true;
	hit.Distance = result.mFraction * maxDist;
	hit.Entity = (uint64_t)bi.GetUserData(result.mBodyID);

	JPH::RVec3 point = ray.GetPointOnRay(result.mFraction);
	hit.Point = Vec3(point.GetX(), point.GetY(), point.GetZ());

	// Surface normal at the hit point.
	JPH::BodyLockRead lock(m_Impl->System.GetBodyLockInterfaceNoLock(),
		result.mBodyID);
	if(lock.Succeeded()) {
		const JPH::Body& body = lock.GetBody();
		JPH::Vec3 n = body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point);
		hit.Normal = Vec3(n.GetX(), n.GetY(), n.GetZ());
	}

	return hit;
}

uint32_t PhysicsWorld::AddTriggerCallback(const TriggerCallback& callback) {
	std::lock_guard<std::mutex> lock(m_Impl->CallbackMutex);
	uint32_t token = m_Impl->NextToken++;
	m_Impl->Callbacks.emplace_back(token, callback);
	return token;
}

void PhysicsWorld::RemoveTriggerCallback(uint32_t token) {
	std::lock_guard<std::mutex> lock(m_Impl->CallbackMutex);
	auto& cbs = m_Impl->Callbacks;
	for(auto it = cbs.begin(); it != cbs.end(); ++it) {
		if(it->first == token) {
			cbs.erase(it);
			return;
		}
	}
}

}
