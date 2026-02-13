#pragma once

#include <VolcaniCore/Core/Time.h>

using namespace VolcaniCore;

namespace Lava {

extern void InitComponents();
extern void LoadComponent(const std::string& path);
extern void CloseComponents();
extern void BeginFrame();
extern void Update(TimeStep ts);
extern void EndFrame();

}