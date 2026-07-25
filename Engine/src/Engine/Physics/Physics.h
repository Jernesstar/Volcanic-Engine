#pragma once

namespace VolcanicEngine::Physics {

// Global Jolt process-wide lifetime: registers the default allocator, creates
// the type Factory, and registers Jolt's shape/collision types. Must be called
// once before any PhysicsWorld is created and torn down once at shutdown.
void Init();
void Close();

}
