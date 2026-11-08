#pragma once
#include "Defines.h"

namespace Wonder
{
    class PrefabAsset;
}

namespace Goose
{
    struct TransformComponent;
}

namespace ECS
{
    EntityID Instantiate(Wonder::PrefabId aPrefabId,const Goose::TransformComponent& aTransform);
    void Instantiate(const Wonder::PrefabAsset& aPrefab,const Goose::TransformComponent& aTransform,EntityID aHeadEntityId);
}
