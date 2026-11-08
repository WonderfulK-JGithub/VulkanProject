#include "ECSLoading.h"

#include "Entity.h"
#include "AssetManagement/AssetRegistry.h"
#include "AssetManagement/PrefabAsset.h"
#include "Reflection/ComponentType.h"

namespace ECS
{
    static void InstantiateChildren(const Wonder::PrefabAsset& aPrefab, const EntityID aHeadEntityId,
        const glm::mat4x4& aTransform, const EntityID aParentEntityID, const Wonder::PrefabObjectId aParentObjectId)
    {
        auto& prefabObjects = aPrefab.GetPrefabObjects();
        auto& parentToChildMap = aPrefab.GetParentToChildrenMap();
        
        auto& children = parentToChildMap.at(aParentObjectId);

        for (auto& childObjectId : children)
        {
            const Wonder::PrefabObjectData& childObjectData = prefabObjects.at(childObjectId);
            
            const bool isStatic            = false;
            const Wonder::LayerIndex layer = 0;

            const EntityID childEntityId = ECS::CreateEntity(isStatic, layer).GetID();
            globalCurrentECS->referenceToIdMap.emplace(ECS::EntityReference{.headEntityId = aHeadEntityId,.prefabObjectId = childObjectId},childEntityId);
            AssignParentToEntity(childEntityId,aParentEntityID);
            
            const glm::mat4x4 childTransform = aTransform * glm::mat4x4(Goose::TransformComponent(childObjectData.transform));
            *ECS::Entity(childEntityId).GetTransform() = childTransform;
            
            for (const Wonder::ComponentData& componentData : childObjectData.components)
            {
                componentData.GetType()->OnGameComponentCreate(ECS::Entity(childEntityId), componentData.AccessData());
            }
            
            if (parentToChildMap.contains(childObjectId))
            {
                InstantiateChildren(aPrefab,aHeadEntityId,childTransform,childEntityId,childObjectId);
            }
        }
        
        
    }
    
    EntityID Instantiate(const Wonder::PrefabId aPrefabId, const Goose::TransformComponent& aTransform)
    {
        Wonder::PrefabAsset& prefab = Wonder::GetPrefabRegistry().AccessAssetFromID(aPrefabId);

        const EntityID id = Internal::globalNextFreeEntityID++;
        Instantiate(prefab,aTransform,id);
        
        return id;
    }

    void Instantiate(const Wonder::PrefabAsset& aPrefab, const Goose::TransformComponent& aTransform, EntityID aHeadEntityId)
    {
        const auto& prefabObjects = aPrefab.GetPrefabObjects();
        const Wonder::PrefabObjectData& headObjectData = prefabObjects.at(aPrefab.GetHeadObjectId());

        const bool isStatic            = false;
        const Wonder::LayerIndex layer = 0;

        ECS::CreateEntity(isStatic, layer,aHeadEntityId).GetID();
        
        {
            *ECS::Entity(aHeadEntityId).GetTransform() = aTransform;
            
            for (const Wonder::ComponentData& componentData : headObjectData.components)
            {
                componentData.GetType()->OnGameComponentCreate(ECS::Entity(aHeadEntityId), componentData.AccessData());
            }
        }
        
        if (prefabObjects.size() > 1)
        {
            InstantiateChildren(aPrefab,aHeadEntityId,aTransform,aHeadEntityId,aPrefab.GetHeadObjectId());
        }
    }
}


