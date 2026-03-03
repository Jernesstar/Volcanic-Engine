#pragma once

#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Buffer.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

struct CubemapSpec {

};

class Cubemap : public Derivable<Cubemap> {
public:
	const CubemapSpec Spec;

public:
	Cubemap(const CubemapSpec& spec)
		: Spec(spec) { }

	virtual void SetData(const void* data) = 0;

	template<typename T>
	void SetData(const Buffer<T>& buffer) {
		SetData(buffer.Get());
	}
};

}