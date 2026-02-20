#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Buffer.h>

#include "BufferLayout.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

struct StorageBufferSpec {
	BufferLayout Layout;
	u64 Count = 1;
};

class StorageBuffer : public Derivable<StorageBuffer> {
public:
	const StorageBufferSpec Spec;

public:
	StorageBuffer(const StorageBufferSpec& spec)
		: Spec(spec) { }
	virtual ~StorageBuffer() = default;

	template<typename T>
	void SetData(const Buffer<T>& buffer, u64 offset = 0) {
		VOLCANICORE_ASSERT(sizeof(T) == Spec.Layout.Stride);
		SetData(buffer.Get(), buffer.GetCount(), offset);
	}

	virtual void SetData(const void* data, u64 count = 1, u64 offset = 0) = 0;
};

}