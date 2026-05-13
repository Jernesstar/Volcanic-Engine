#pragma once

#include <VolcaniCore/Core/TimeUtils.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/UUID.h>
#include <VolcaniCore/Core/Log.h>

#include "Utils/TypeID.h"

#include "Entity.h"
#include "EntityBuilder.h"
#include "System.h"

using namespace VolcaniCore;

namespace VolcanicEngine::ECS {

struct Event { };

class World {
public:
	World();
	~World() = default;

	void RegisterComponent();
	void Reset();

	void OnUpdate(TimeStep ts);

	Entity GetEntity(const std::string& name);
	Entity GetEntity(u64 id);

	EntityBuilder BuildEntity() { return EntityBuilder{ *this }; }
	EntityBuilder BuildEntity(u64 id) {
		return EntityBuilder{ *this, id };
	}
	EntityBuilder BuildEntity(const std::string& name) {
		return EntityBuilder{ *this, name };
	}

	Entity AddEntity();
	Entity AddEntity(u64 id);
	Entity AddEntity(const std::string& name);

	void RemoveEntity(u64 id);
	void RemoveEntity(const std::string& name);

	void ForEach(const Func<void, Entity&>& func);
	void ForEach(const Func<void, const Entity&>& func) const;

	template<typename ...TComponents>
	void ForEach(const Func<void, Entity&>& func) {
		flecs::query<TComponents...> q = 
			m_World.query_builder<TComponents...>().build();

		m_World.defer_begin();
		q.each(
			[func](flecs::entity handle, TComponents&...)
			{
				Entity entity{ handle };
				func(entity);
			});
		m_World.defer_end();
	}

	template<class TSystem>
	void Add() {
 		u64 id = TypeIDGenerator<System<>>::GetID<TSystem>();
		m_Systems[id] = new TSystem(this);
	}

	template<typename TSystem>
	TSystem* Get() {
		u64 id = TypeIDGenerator<System<>>::GetID<TSystem>();
		if(!m_Systems.size() || !m_Systems.count(id))
			return nullptr;

		return (TSystem*)m_Systems[id];
	}

	template<class TSystem>
	void Remove() {
 		u64 id = TypeIDGenerator<System<>>::GetID<TSystem>();
		if(!m_Systems.size() || !m_Systems.count(id))
			return;

		delete (TSystem*)m_Systems[id];
		m_Systems.erase(id);
	}

	flecs::world& GetNative() { return m_World; }

private:
	flecs::world m_World;
	flecs::query<> m_AllEntitiesQuery;
	Map<u64, void*> m_Systems;
};

}