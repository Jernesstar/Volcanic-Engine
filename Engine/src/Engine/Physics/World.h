#pragma once

#include <cstdint>
#include <memory>

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/List.h>

#include "Shape.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Physics {

// Object layers: statics never collide with statics; sensors overlap everything
// but never solve contacts (static geometry + sensors only).
namespace Layers {
	static constexpr uint16_t NON_MOVING = 0;
	static constexpr uint16_t MOVING	 = 1;
	static constexpr uint16_t SENSOR	 = 2;
	static constexpr uint16_t NUM_LAYERS = 3;
}

// A single raycast hit against a body in the world. Entity is the raw ECS id
// stored as body user data; a value of 0 with HasHit=false means no hit.
struct RayHit {
	bool HasHit = false;
	uint64_t Entity = 0;
	float Distance = 0.0f;
	Vec3 Point = { 0.0f, 0.0f, 0.0f };
	Vec3 Normal = { 0.0f, 0.0f, 0.0f };
};

// Emitted by the contact listener when a sensor body's overlap set changes.
struct TriggerEvent {
	uint64_t Trigger = 0; // The sensor body's entity.
	uint64_t Other = 0;   // The entity that entered/exited.
	bool Enter = false;   // true = enter, false = exit.
};

using TriggerCallback = std::function<void(const TriggerEvent&)>;

// Opaque handle wrapping a JPH::BodyID (see World.cpp). We hide the Jolt type
// here so downstream translation units that only need to reference bodies do
// not have to include Jolt headers.
struct BodyHandle {
	uint32_t ID = 0xFFFFFFFF; // JPH::BodyID::cInvalidBodyID
	bool IsValid() const { return ID != 0xFFFFFFFF; }
};

// Real Jolt-backed physics world. Owns the JPH::PhysicsSystem, layer
// interfaces, temp allocator, job system and contact listener. Scoped to
// query-only usage (static colliders + sensors); no rigid-body dynamics.
class PhysicsWorld {
public:
	static const float StepSize; // Fixed physics timestep (1/60 s).

public:
	PhysicsWorld();
	~PhysicsWorld();

	PhysicsWorld(const PhysicsWorld&) = delete;
	PhysicsWorld& operator =(const PhysicsWorld&) = delete;

	// Advance the simulation with a fixed-timestep accumulator. Multiple
	// sub-steps run if dt exceeds StepSize.
	void Step(float dt);

	void SetGravity(const Vec3& gravity);
	Vec3 GetGravity() const;

	// Add a static (non-moving) collider. `entity` is the raw ECS id stored as
	// body user data so queries and trigger events can resolve back to it.
	BodyHandle AddStaticBody(const Ref<Shape>& shape, const Transform& transform,
		uint64_t entity);

	// Add a sensor (trigger) collider. Sensors report enter/exit overlaps via
	// the contact listener but never solve contacts.
	BodyHandle AddSensorBody(const Ref<Shape>& shape, const Transform& transform,
		uint64_t entity);

	void RemoveBody(BodyHandle handle);
	void SetBodyTransform(BodyHandle handle, const Transform& transform);
	Transform GetBodyTransform(BodyHandle handle) const;

	// Rebuild the broadphase acceleration structure. Newly added bodies are not
	// visible to raycast/overlap queries until the broadphase is optimized (or
	// the world is stepped), so call this after a batch of AddStaticBody /
	// AddSensorBody calls if you need to query before the next Step.
	void OptimizeBroadPhase();

	// Nearest hit along the ray within maxDist. `layerMask` is a bitmask over
	// Layers::* (bit N = layer N); default hits everything. Sensors are only
	// hit if their layer bit is set (they are excluded by default).
	RayHit Raycast(const Vec3& origin, const Vec3& direction,
		float maxDist = 1000.0f, uint32_t layerMask = 0xFFFFFFFF) const;

	// Register a callback invoked for every trigger enter/exit event. Returns a
	// token usable to unsubscribe.
	uint32_t AddTriggerCallback(const TriggerCallback& callback);
	void RemoveTriggerCallback(uint32_t token);

private:
	struct Impl;
	std::unique_ptr<Impl> m_Impl;
};

}
