#pragma once

#include <VolcaniCore/Core/Time.h>

using namespace VolcaniCore;

namespace Pyro {

extern void Init();
extern void Close();
extern void RegisterInterface();
extern void BeginFrame();
extern void Update(TimeStep ts);
extern void EndFrame();

}