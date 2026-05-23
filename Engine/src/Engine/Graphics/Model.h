#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include <Engine/Asset/AssetRegistry.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

struct ModelNode {
	Transform LocalTransform;
	Asset GeometryAsset;
	Asset MaterialAsset;
	List<ModelNode> Children;
};

struct Model {
	// Str Name;
	ModelNode Root;
};

}