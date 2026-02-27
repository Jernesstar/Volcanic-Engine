#include "AssetRegistry.h"

#include <VolcaniCore/Core/Template.h>

namespace VolcanicEngine {

class AssetManager : public VolcaniCore::Derivable<AssetManager> {
public:
	static AssetManager* Get() { return s_Instance; }

public:
	AssetManager() { s_Instance = this; }

	template<typename T>
	Ref<T> Load(Asset asset) = 0;

	Ref<AssetRegistry> GetRegistry() const { return m_AssetRegistry; }

protected:
	Ref<AssetRegistry> m_AssetRegistry;

private:
	static AssetManager* s_Instance;
};


}