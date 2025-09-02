#include "World.h"

#include <VolcaniCore/Core/UUID.h>

namespace Lava::ECS {

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

Entity World::GetEntity(VolcaniCore::UUID id) {
	return { m_World.entity((uint64_t)id) };
}

Entity World::AddEntity() {
	return { m_World.entity() };
}

Entity World::AddEntity(VolcaniCore::UUID id) {
	return { m_World.make_alive((uint64_t)id) };
}

Entity World::AddEntity(const std::string& name) {
	return { m_World.entity(name.c_str()) };
}

void World::RemoveEntity(VolcaniCore::UUID id) {
	GetEntity(id).Kill();
}

void World::RemoveEntity(const std::string& name) {
	GetEntity(name).Kill();
}

void World::ForEach(const Func<void, Entity&>& func) {
	m_World.defer_begin();

	m_AllEntitiesQuery.each(
		[&func](flecs::entity handle)
		{
			Entity entity{ handle };
			func(entity);
		});

	m_World.defer_end();
}

void World::ForEach(const Func<void, const Entity&>& func) const {
	m_World.defer_begin();

	m_AllEntitiesQuery.each(
		[&func](flecs::entity handle)
		{
			Entity entity{ handle };
			func(entity);
		});

	m_World.defer_end();
}

}