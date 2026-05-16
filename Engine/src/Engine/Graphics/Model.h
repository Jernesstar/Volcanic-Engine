#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include <Engine/Asset/AssetRegistry.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

struct ModelNode {
	Str             Name;
	Transform       LocalTransform;
	Asset           GeometryAsset;   // may be null (group node)
	List<Asset>     MaterialAssets;  // one per surface slot
	List<ModelNode> Children;
};

struct ModelAsset {
	ModelNode Root;
};

}