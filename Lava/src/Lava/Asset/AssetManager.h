#include "AssetRegistry.h"

#include <VolcaniCore/Core/Template.h>

namespace Lava {

class AssetManager : public VolcaniCore::Derivable<AssetManager> {
public:
	static AssetManager* Get() { return s_Instance; }

public:
	void LoadRegistry();

private:
	static AssetManager* s_Instance;
};


}