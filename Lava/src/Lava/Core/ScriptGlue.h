#pragma once

#include "Script/ScriptObject.h"

using namespace Lava::Script;

namespace Lava {

class ScriptGlue {
public:
	static void RegisterInterface();
	static void Copy(Ref<ScriptObject> src, Ref<ScriptObject> dst);
};

}