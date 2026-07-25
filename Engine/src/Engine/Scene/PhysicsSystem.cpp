#include "PhysicsSystem.h"

#include <algorithm>

#include <VolcaniCore/Core/Log.h>

#include "ECS/World.h"

#include "Physics/Physics.h"
#include "Physics/Shape.h"

#include "Asset/AssetManager.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::ECS;

namespace VolcanicEngine {

PhysicsSystem::PhysicsSystem(ECS::World* world)
	: System(world)
{
	// Jolt global init/teardown is owned by m_World (Physics::PhysicsWorld).
}

PhysicsSystem::~PhysicsSystem() {
	// Bodies are owned by m_World, which tears down cleanly on destruction.
	m_Bodies.clear();
}

Entity PhysicsSystem::ResolveEntity(uint64_t id) {
	if(id == 0)
		return Entity{};
	return m_World3D->GetEntity(id);
}

// Build a Jolt collider for an entity's RigidBodyComponent. Box shapes come
// from HalfExtents; mesh shapes are cooked from the entity's MeshComponent
// geometry (falling back to a box AABB if the geometry can't be resolved).
void PhysicsSystem::BuildBody(Entity& entity) {
	if(!entity.IsValid() || !entity.Has<RigidBodyComponent>()
	|| !entity.Has<TransformComponent>())
		return;

	BuildBody(entity.GetHandle().id(), entity.Get<RigidBodyComponent>(),
		entity.Get<TransformComponent>(),
		entity.Has<MeshComponent>() ? &entity.Get<MeshComponent>() : nullptr);
}

void PhysicsSystem::BuildBody(uint64_t id, const RigidBodyComponent& rb,
	const TransformComponent& tr, const MeshComponent* mc)
{
	RemoveBody(id); // Rebuild if it already existed.

	Ref<Physics::Shape> shape;
	if(rb.Shape == RigidBodyComponent::ShapeType::Mesh && mc) {
		Ref<Graphics::Geometry> geo;
		if(auto* am = AssetManager::Get())
			geo = am->Get<Graphics::Geometry>(mc->GeometryAsset);

		if(geo)
			shape = Physics::Shape::CreateMesh(geo);
	}

	if(!shape) {
		// Box: HalfExtents scaled by the entity transform (matches the old AABB
		// collider semantics).
		Vec3 half = rb.HalfExtents * tr.Scale;
		switch(rb.Shape) {
			case RigidBodyComponent::ShapeType::Sphere:
				shape = Physics::Shape::CreateSphere(
					std::max({ half.x, half.y, half.z }));
				break;
			case RigidBodyComponent::ShapeType::Capsule:
				shape = Physics::Shape::CreateCapsule(
					half.y, std::max(half.x, half.z));
				break;
			default:
				shape = Physics::Shape::CreateBox(half);
				break;
		}
	}

	if(!shape)
		return;

	// The physics transform uses world translation/rotation; box half-extents
	// already fold in scale above, so use unit scale in the pose.
	Transform pose;
	pose.Translation = tr.Translation;
	pose.Rotation = tr.Rotation;

	Physics::BodyHandle body = rb.IsTrigger
		? m_World.AddSensorBody(shape, pose, id)
		: m_World.AddStaticBody(shape, pose, id);

	if(body.IsValid())
		m_Bodies[id] = body;
}

void PhysicsSystem::RemoveBody(uint64_t id) {
	auto it = m_Bodies.find(id);
	if(it == m_Bodies.end())
		return;
	m_World.RemoveBody(it->second);
	m_Bodies.erase(it);
}

// Build Jolt colliders for any RigidBodyComponent entities not yet mirrored.
// Called before every step and every query so colliders are live even if a
// query runs before the first physics step (e.g. from a script's setup).
void PhysicsSystem::SyncBodies() {
	size_t before = m_Bodies.size();

	// Iterate the flecs query directly (not through the deferred World::ForEach)
	// and read the collider/transform straight from the table columns. This
	// avoids a stale/garbage component read that occurred when re-fetching the
	// RigidBodyComponent via Entity::Get inside the deferred query callback.
	auto& world = m_World3D->GetNative();
	auto q = world.query_builder<const RigidBodyComponent,
		const TransformComponent>().build();

	q.each(
		[&](flecs::entity handle, const RigidBodyComponent& rb,
			const TransformComponent& tr)
		{
			uint64_t id = handle.id();
			if(m_Bodies.count(id))
				return;

			const MeshComponent* mc =
				handle.has<MeshComponent>() ? &handle.get<MeshComponent>()
											: nullptr;
			BuildBody(id, rb, tr, mc);
		});

	// Newly added bodies are not visible to queries until the broadphase is
	// rebuilt (Jolt only refreshes it on Step). Do it once per batch.
	if(m_Bodies.size() != before)
		m_World.OptimizeBroadPhase();
}

void PhysicsSystem::Update(TimeStep ts) {
	SyncBodies();
	m_World.Step((f32)ts);
}

void PhysicsSystem::Run(Entity& entity, TimeStep ts, Phase phase) { }

void PhysicsSystem::OnComponentAdd(Entity& entity) {
	BuildBody(entity);
}

void PhysicsSystem::OnComponentSet(Entity& entity) {
	BuildBody(entity);
}

void PhysicsSystem::OnComponentRemove(Entity& entity) {
	RemoveBody(entity.GetHandle().id());
}

HitInfo PhysicsSystem::Raycast(const Vec3& origin, const Vec3& direction,
	f32 maxDist)
{
	SyncBodies();

	HitInfo out;
	Physics::RayHit hit = m_World.Raycast(origin, direction, maxDist);
	if(!hit.HasHit)
		return out;

	out.HasHit = true;
	out.Distance = hit.Distance;
	out.Point = hit.Point;
	out.Normal = hit.Normal;
	out.HitEntity = ResolveEntity(hit.Entity);
	return out;
}

Entity PhysicsSystem::OverlapPoint(const Vec3& point) {
	SyncBodies();

	// Overlap query via a downward ray through the point from above. Sensors
	// (triggers) ARE included here (overlap is exactly how triggers are meant to
	// be detected). Returns the entity whose collider the column at this XZ
	// passes through at/around the query height.
	const float above = 10.0f;
	const float range = above + 10.0f;
	// All layers including sensors (bits for NON_MOVING, MOVING, SENSOR).
	const uint32_t mask =
		(1u << Physics::Layers::NON_MOVING)
		| (1u << Physics::Layers::MOVING)
		| (1u << Physics::Layers::SENSOR);

	Physics::RayHit hit = m_World.Raycast(
		point + Vec3(0.0f, above, 0.0f), Vec3(0.0f, -1.0f, 0.0f), range, mask);

	// Only count it as an overlap if the hit is at or below the query point (the
	// ray reached down to the point's height inside the collider's span).
	if(hit.HasHit && hit.Point.y >= point.y - 1e-3f)
		return ResolveEntity(hit.Entity);
	return Entity{};
}

uint32_t PhysicsSystem::SubscribeTrigger(const ScriptTriggerCallback& callback) {
	return m_World.AddTriggerCallback(
		[this, callback](const Physics::TriggerEvent& ev) {
			TriggerInfo info;
			info.Trigger = ResolveEntity(ev.Trigger);
			info.Other = ResolveEntity(ev.Other);
			info.Enter = ev.Enter;
			callback(info);
		});
}

void PhysicsSystem::UnsubscribeTrigger(uint32_t token) {
	m_World.RemoveTriggerCallback(token);
}

}
