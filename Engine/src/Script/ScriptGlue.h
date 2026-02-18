#pragma once

#include "Script/ScriptObject.h"

using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

class ScriptGlue {
public:
	static void RegisterInterface();
	static void Copy(Ref<ScriptObject> src, Ref<ScriptObject> dst);
};

}