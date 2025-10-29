#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>

using namespace VolcaniCore;

namespace Magma {

class Integration : public Derivable<Integration> {
public:
	virtual ~Integration() = default;

	virtual void Init() = 0;
	virtual void Shutdown() = 0;
};

}