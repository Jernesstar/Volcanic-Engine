#include "GameSystemRegistry.h"

#include <cmath>

#include <angelscript.h>

#include "Script/ScriptEngine.h"

using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

GameSystemRegistry::~GameSystemRegistry() {
	Clear();
}

void GameSystemRegistry::Add(asIScriptObject* obj, float interval) {
	if(!obj)
		return;

	obj->AddRef();

	Entry entry;
	entry.Object = obj;
	entry.Interval = interval;
	entry.Accumulator = 0.0f;

	auto* type = obj->GetObjectType();
	entry.OnAttach = type->GetMethodByDecl("void OnAttach()");
	entry.OnUpdate = type->GetMethodByDecl("void OnUpdate(float, float)");
	entry.OnTick   = type->GetMethodByDecl("void OnTick(float)");

	m_Entries.Add(entry);

	if(entry.OnAttach) {
		ScriptFunc func{ entry.OnAttach, ScriptEngine::GetHookContext(),
			entry.Object };
		func.CallVoid();
	}
}

void GameSystemRegistry::Remove(asIScriptObject* obj) {
	for(u64 i = 0; i < m_Entries.Count(); i++) {
		if(m_Entries[i].Object == obj) {
			obj->Release();
			m_Entries.Pop(i);
			return;
		}
	}
}

void GameSystemRegistry::Clear() {
	for(u64 i = 0; i < m_Entries.Count(); i++) {
		if(m_Entries[i].Object)
			m_Entries[i].Object->Release();
	}
	m_Entries.Clear();
}

void GameSystemRegistry::Update(TimeStep ts) {
	float dt = (float)ts;

	for(u64 i = 0; i < m_Entries.Count(); i++) {
		auto& entry = m_Entries[i];
		float alpha = 0.0f;

		if(entry.OnTick && entry.Interval > 0.0f) {
			entry.Accumulator += dt;

			u32 ticks = 0;
			while(entry.Accumulator >= entry.Interval
			   && ticks < s_MaxCatchUpTicks) {
				ScriptFunc func{ entry.OnTick, ScriptEngine::GetHookContext(),
					entry.Object };
				func.CallVoid(entry.Interval);
				entry.Accumulator -= entry.Interval;
				ticks++;
			}

			// Long stall: drop the leftover backlog so a hitch cannot spiral
			// into an unbounded tick storm on the next frame.
			if(entry.Accumulator >= entry.Interval)
				entry.Accumulator = std::fmod(entry.Accumulator, entry.Interval);

			alpha = entry.Accumulator / entry.Interval;
		}

		if(entry.OnUpdate) {
			ScriptFunc func{ entry.OnUpdate, ScriptEngine::GetHookContext(),
				entry.Object };
			func.CallVoid(dt, alpha);
		}
	}
}

}
