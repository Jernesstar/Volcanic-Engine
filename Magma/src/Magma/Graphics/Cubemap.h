#pragma once

#include <VolcaniCore/Core/List.h>

#include "Texture.h"

using namespace VolcaniCore;

namespace Magma::Graphics {

class Cubemap : public Derivable<Cubemap> {
public:
	static Ref<Cubemap> Create(const List<ImageData>& faces);

public:
	Cubemap() = default;
	virtual ~Cubemap() = default;
};

}