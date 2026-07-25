#include "Physics.h"

#include <cstdarg>
#include <cstdio>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>

#include <VolcaniCore/Core/Log.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Physics {

static bool s_Initialized = false;
static int s_RefCount = 0;

static void TraceImpl(const char* fmt, ...) {
	char buffer[1024];
	va_list list;
	va_start(list, fmt);
	std::vsnprintf(buffer, sizeof(buffer), fmt, list);
	va_end(list);
	Log::Info("[Jolt] {}", buffer);
}

void Init() {
	s_RefCount++;
	if(s_Initialized)
		return;

	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;

	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	s_Initialized = true;
}

void Close() {
	if(!s_Initialized)
		return;
	if(--s_RefCount > 0)
		return;

	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	s_Initialized = false;
}

}
