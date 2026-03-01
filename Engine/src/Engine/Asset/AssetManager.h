#pragma once

#include "AssetRegistry.h"

#include <VolcaniCore/Core/Template.h>

namespace VolcanicEngine {

class AssetManager : public VolcaniCore::Derivable<AssetManager> {
public:
	static AssetManager* Get() { return s_Instance; }

public:
	AssetManager() { s_Instance = this; }

	virtual void Load(Asset asset) = 0;
	virtual void Unload(Asset asset) = 0;

	template<typename T>
	Ref<T> Get(Asset asset) {
		if(!m_AssetRegistry->IsLoaded(asset))
			Load(asset);

		// Bytes bytes = m_AssetRegistry->Get(asset);
		// return AssetData<T>::CreateRef(bytes);
		return nullptr;
	}

	template<typename T>
	Ref<T> Get(const std::string& name) {
		Asset asset = m_AssetRegistry->FindAsset(name);
		return Get<T>(asset);
	}

	Ref<AssetRegistry> GetRegistry() const { return m_AssetRegistry; }

protected:
	Ref<AssetRegistry> m_AssetRegistry;

private:
	inline static AssetManager* s_Instance;
};


}