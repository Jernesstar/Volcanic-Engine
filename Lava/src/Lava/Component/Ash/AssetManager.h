#include "AssetRegistry.h"

#include <VolcaniCore/Core/Template.h>

namespace Ash {

enum class Event {
	AssetAdded,
	AssetRemoved,
	AssetLoaded,
	AssetUnloaded,
	AssetUpdated,
	RegistryLoaded,
	RegistryUnloaded
};

class AssetManager : public VolcaniCore::Derivable<AssetManager> {
public:
	static AssetManager* Get() { return s_Instance; }

public:
	void LoadRegistry();

private:
	static AssetManager* s_Instance;
};


}