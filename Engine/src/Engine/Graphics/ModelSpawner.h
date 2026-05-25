#pragma once

#include <Engine/ECS/World.h>
#include <Engine/ECS/Entity.h>
#include <Engine/Graphics/Model.h>
#include <Engine/Asset/AssetManager.h>
#include <Engine/Scene/Component.h>

using namespace VolcaniCore;
using namespace VolcanicEngine::ECS;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

namespace Detail {

static Entity SpawnModelNode(
    World& world, const ModelNode& node, Entity parent)
{
    Entity entity = world.AddEntity();
    entity.Add<TransformComponent>(node.LocalTransform);

    // Only attach a mesh when both geometry and material were resolved
    if(node.GeometryAsset && node.MaterialAsset)
        entity.Add<MeshComponent>(node.GeometryAsset, node.MaterialAsset);

    if(parent)
        entity.GetHandle().child_of(parent.GetHandle());

    for(auto& child : node.Children)
        SpawnModelNode(world, child, entity);

    return entity;
}

} // namespace Detail

// Spawn the full entity hierarchy for a model asset.
// The returned entity is the root; geometry entities are its descendants.
inline Entity SpawnModel(World& world, Asset modelAsset) {
    auto model = AssetManager::Get()->Get<Model>(modelAsset);
    if(!model) return {};
    return Detail::SpawnModelNode(world, model->Root, {});
}

inline Entity SpawnModel(World& world, const Str& modelName) {
    Asset asset = AssetManager::Get()->GetRegistry()->FindAsset(modelName);
    return SpawnModel(world, asset);
}

} // namespace VolcanicEngine
