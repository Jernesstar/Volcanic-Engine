#pragma once

#include <VolcaniCore/Core/Math.h>

#include "ECS/System.h"
#include "ECS/Entity.h"

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

// Result of a raycast against the scene's AABB colliders.
struct HitInfo {
	bool HasHit = false;
	Entity HitEntity;
	Vec3 Point = { 0.0f, 0.0f, 0.0f };
	f32 Distance = 0.0f;

	operator bool() const { return HasHit; }
};

// Minimal, query-only physics: raycasts and overlap tests against static
// axis-aligned box colliders (RigidBodyComponent). No rigid-body dynamics.
class PhysicsSystem : public System<RigidBodyComponent> {
public:
	PhysicsSystem(ECS::World* world);

	void Update(TimeStep ts) override;
	void Run(Entity& entity, TimeStep ts, Phase phase) override;

	void OnComponentAdd(Entity& entity) override;
	void OnComponentSet(Entity& entity) override;
	void OnComponentRemove(Entity& entity) override;

	// Cast a ray and return the nearest collider hit within maxDist. Triggers
	// are ignored by raycasts (they are reported by OverlapPoint).
	HitInfo Raycast(const Vec3& origin, const Vec3& direction,
		f32 maxDist = 1000.0f);

	// Return the first collider whose box contains the point (an invalid Entity
	// if none). Useful as a lightweight trigger/overlap query.
	Entity OverlapPoint(const Vec3& point);
};

}
