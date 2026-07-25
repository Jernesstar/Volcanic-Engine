#pragma once

// The rigid-body dynamics wrapper (formerly PhysX-based) is intentionally
// removed. Sprint 60 scopes physics to static colliders + sensor/trigger
// volumes via Physics::PhysicsWorld (see World.h). Dynamics are deferred until
// a level design needs them (see the sprint "Out of scope" section).
