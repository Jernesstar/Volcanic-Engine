#include "AssetManager.h"

namespace VolcanicEngine {

// General implementation (optional: can throw error if type is unsupported)
template<typename T>
Ref<T> LoadFromBytes(Bytes&& bytes) {
	static_assert(sizeof(T) == 0, "Specify a specialization for this type!");
	return nullptr;
}

// Explicit Specialization for Mesh
template<>
Ref<Mesh> LoadFromBytes<Mesh>(Bytes&& bytes) {
	return nullptr;
}

template Ref<Mesh> LoadFromBytes<Mesh>(Bytes&&);

}