#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#include <VolcaniCore/Core/Math.h>

#include "ECS/System.h"
#include "ECS/Entity.h"

#include "Physics/World.h"

#include "Component.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::ECS;

namespace VolcanicEngine {

struct PhysicsEvent {
	PhysicsEvent() = default;
	virtual ~PhysicsEvent() = default;
};

struct CollisionEvent : public PhysicsEvent {
	Entity Other;

	CollisionEvent(Entity entity)
		: Other(entity) { }
};

struct ClickedEvent : public PhysicsEvent {

};

// Result of a raycast against the physics world.
struct HitInfo {
	bool HasHit = false;
	Entity HitEntity;
	Vec3 Point = { 0.0f, 0.0f, 0.0f };
	Vec3 Normal = { 0.0f, 0.0f, 0.0f };
	f32 Distance = 0.0f;

	operator bool() const { return HasHit; }
};

// Trigger enter/exit event resolved into scene entities.
struct TriggerInfo {
	Entity Trigger; // The sensor entity.
	Entity Other;   // The entity that entered/exited.
	bool Enter = false;
};

using ScriptTriggerCallback = std::function<void(const TriggerInfo&)>;

// Physics system backed by a real Jolt Physics::PhysicsWorld. Builds static
// colliders and sensor volumes from RigidBodyComponents, steps the world each
// frame during play, and exposes raycast + overlap + trigger queries. The
// public Raycast/OverlapPoint API surface is kept stable so downstream
// systems consume it unchanged.
class PhysicsSystem : public System<RigidBodyComponent> {
public:
	PhysicsSystem(ECS::World* world);
	~PhysicsSystem();

	void Update(TimeStep ts) override;
	void Run(Entity& entity, TimeStep ts, Phase phase) override;

	void OnComponentAdd(Entity& entity) override;
	void OnComponentSet(Entity& entity) override;
	void OnComponentRemove(Entity& entity) override;

	// Cast a ray and return the nearest collider hit within maxDist. Sensors
	// (triggers) are excluded from raycasts by default.
	HitInfo Raycast(const Vec3& origin, const Vec3& direction,
		f32 maxDist = 1000.0f);

	// Return the first collider whose volume contains the point (an invalid
	// Entity if none). Lightweight overlap/trigger query.
	Entity OverlapPoint(const Vec3& point);

	// Subscribe to trigger enter/exit events. Returns a token to unsubscribe.
	uint32_t SubscribeTrigger(const ScriptTriggerCallback& callback);
	void UnsubscribeTrigger(uint32_t token);

	Physics::PhysicsWorld& GetWorld() { return m_World; }

private:
	Physics::PhysicsWorld m_World;

	// entity id -> its Jolt body, so we can rebuild/remove on component changes.
	std::unordered_map<uint64_t, Physics::BodyHandle> m_Bodies;

	Entity ResolveEntity(uint64_t id);
	void SyncBodies();
	void BuildBody(Entity& entity);
	void BuildBody(uint64_t id, const RigidBodyComponent& rb,
		const TransformComponent& tr, const MeshComponent* mc);
	void RemoveBody(uint64_t id);
};

}
