#include "PhysicsSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <VolcaniCore/Core/Log.h>

#include "ECS/World.h"

using namespace VolcanicEngine::ECS;

namespace VolcanicEngine {

PhysicsSystem::PhysicsSystem(ECS::World* world)
	: System(world)
{

}

// Query-only system: nothing to step each frame.
void PhysicsSystem::Update(TimeStep ts) { }
void PhysicsSystem::Run(Entity& entity, TimeStep ts, Phase phase) { }
void PhysicsSystem::OnComponentAdd(Entity& entity) { }
void PhysicsSystem::OnComponentSet(Entity& entity) { }
void PhysicsSystem::OnComponentRemove(Entity& entity) { }

// Slab-test ray/AABB intersection. `d` must be normalised so `tHit` is a
// distance in world units.
static bool RayAABB(const Vec3& o, const Vec3& d, const Vec3& mn,
	const Vec3& mx, f32& tHit)
{
	f32 tmin = 0.0f;
	f32 tmax = std::numeric_limits<f32>::max();

	for(int i = 0; i < 3; i++) {
		if(std::abs(d[i]) < 1e-8f) {
			if(o[i] < mn[i] || o[i] > mx[i])
				return false;
		}
		else {
			f32 inv = 1.0f / d[i];
			f32 t1 = (mn[i] - o[i]) * inv;
			f32 t2 = (mx[i] - o[i]) * inv;
			if(t1 > t2)
				std::swap(t1, t2);
			tmin = std::max(tmin, t1);
			tmax = std::min(tmax, t2);
			if(tmin > tmax)
				return false;
		}
	}

	tHit = tmin;
	return true;
}

HitInfo PhysicsSystem::Raycast(const Vec3& origin, const Vec3& direction,
	f32 maxDist)
{
	HitInfo best;
	best.Distance = maxDist;

	if(glm::length(direction) < 1e-8f)
		return best;
	Vec3 d = glm::normalize(direction);

	m_World3D->ForEach<RigidBodyComponent, TransformComponent>(
		[&](Entity& e) {
			auto& rb = e.Get<RigidBodyComponent>();
			if(rb.IsTrigger)
				return;

			auto& tr = e.Get<TransformComponent>();
			Vec3 half = rb.HalfExtents * tr.Scale;
			Vec3 mn = tr.Translation - half;
			Vec3 mx = tr.Translation + half;

			f32 t;
			if(RayAABB(origin, d, mn, mx, t) && t >= 0.0f && t <= best.Distance) {
				best.HasHit = true;
				best.Distance = t;
				best.HitEntity = e;
				best.Point = origin + d * t;
			}
		});

	return best;
}

Entity PhysicsSystem::OverlapPoint(const Vec3& point) {
	Entity result;

	m_World3D->ForEach<RigidBodyComponent, TransformComponent>(
		[&](Entity& e) {
			if(result.IsValid())
				return;

			auto& rb = e.Get<RigidBodyComponent>();
			auto& tr = e.Get<TransformComponent>();
			Vec3 half = rb.HalfExtents * tr.Scale;
			Vec3 mn = tr.Translation - half;
			Vec3 mx = tr.Translation + half;

			if(point.x >= mn.x && point.x <= mx.x
			&& point.y >= mn.y && point.y <= mx.y
			&& point.z >= mn.z && point.z <= mx.z)
				result = e;
		});

	return result;
}

}
