#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/TimeUtils.h>

class asIScriptObject;
class asIScriptFunction;

using namespace VolcaniCore;

namespace VolcanicEngine {

// Fixed-tick gameplay systems, run by the engine outside flecs iteration.
//
// A game registers script objects implementing the IGameSystem interface
// (OnAttach / OnUpdate(float ts, float tickAlpha) / OnTick(float dt)). The
// engine owns a per-entry time accumulator: every frame it advances the
// accumulator by the frame's timestep and calls OnTick(interval) once per whole
// interval elapsed (clamped to a small catch-up budget so a hitch cannot spiral
// into an unbounded tick storm), then calls OnUpdate(ts, alpha) once with the
// interpolation fraction alpha = accumulator / interval for smooth rendering
// between ticks. An interval of 0 disables OnTick (OnUpdate still runs, alpha 0).
//
// This mirrors the render-hook registration pattern: entries hold an AddRef'd
// asIScriptObject with its interface methods resolved once and cached. The
// registry is scene-scoped and cleared on scene teardown, which releases the
// script references.
class GameSystemRegistry {
public:
	GameSystemRegistry() = default;
	~GameSystemRegistry();

	// Registers a script object as a gameplay system. interval is the fixed tick
	// period in seconds (0 = no OnTick). OnAttach is called immediately.
	void Add(asIScriptObject* obj, float interval);

	// Removes a previously registered system, releasing its script reference.
	void Remove(asIScriptObject* obj);

	// Releases all held script references and drops all entries.
	void Clear();

	// Advances every system: accumulate, run catch-up ticks, then OnUpdate with
	// the interpolation alpha. Call once per frame, before World3D updates.
	void Update(TimeStep ts);

private:
	// Max whole ticks executed in one frame before the remaining backlog is
	// dropped (via fmod on the accumulator). Prevents a long frame from
	// producing an unbounded catch-up storm.
	static constexpr u32 s_MaxCatchUpTicks = 4;

	struct Entry {
		asIScriptObject* Object = nullptr;
		asIScriptFunction* OnAttach = nullptr;
		asIScriptFunction* OnUpdate = nullptr;
		asIScriptFunction* OnTick = nullptr;
		float Interval = 0.0f;
		float Accumulator = 0.0f;
	};

	List<Entry> m_Entries;
};

}
