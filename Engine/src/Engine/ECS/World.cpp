#include "World.h"

#include <VolcaniCore/Core/UUID.h>

namespace VolcanicEngine::ECS {

struct RealEntity { };

World::World() { }

void World::Reset() {
	List<Entity> all;
	ForEach([&](Entity entity) { all.Add(entity); });
	for(auto entity : all)
		entity.Kill();
}

void World::OnUpdate(TimeStep ts) {
	m_World.progress(ts);
}

Entity World::GetEntity(const std::string& name) {
	return { m_World.lookup(name.c_str()) };
}

Entity World::GetEntity(u64 id) {
	return { m_World.entity(id) };
}

Entity World::AddEntity() {
	return { m_World.entity().add<RealEntity>() };
}

Entity World::AddEntity(u64 id) {
	return { m_World.entity(id).add<RealEntity>() };
}

Entity World::AddEntity(const std::string& name) {
	return { m_World.entity(name.c_str()).add<RealEntity>() };
}

void World::RemoveEntity(u64 id) {
	GetEntity(id).Kill();
}

void World::RemoveEntity(const std::string& name) {
	GetEntity(name).Kill();
}

void World::ForEach(const Func<void, Entity&>& func) {
	m_World.query<RealEntity>().each(
		[&](flecs::entity handle, RealEntity)
		{
			Entity entity{ handle };
			func(entity);
		});
}

void World::ForEach(const Func<void, const Entity&>& func) const {
	m_World.query<RealEntity>().each(
		[&](flecs::entity handle, RealEntity)
		{
			Entity entity{ handle };
			func(entity);
		});
}

}