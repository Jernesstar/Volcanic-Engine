#pragma once

#include <Magma/Script/ScriptObject.h>

using namespace Magma::Script;

namespace Lava {

class ScriptGlue {
public:
	static void RegisterInterface();
	static void Copy(Ref<ScriptObject> src, Ref<ScriptObject> dst);
};

}