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
            const bool isStatic            = false;
            const Wonder::LayerIndex layer = 0;

            const EntityID childEntityId = ECS::CreateEntity(isStatic, layer,childObjectId).GetID();
            globalCurrentECS->referenceToIdMap.insert_or_assign(ECS::EntityReference{.headEntityId = aHeadEntityId,.prefabObjectId = childObjectId},childEntityId);
            AssignParentToEntity(childEntityId,aParentEntityID);
            
            const Wonder::PrefabObjectData& childObjectData = prefabObjects.at(childObjectId);
            
            const glm::mat4x4 childTransform = aTransform * glm::mat4x4(Goose::TransformComponent(childObjectData.localTransform));
            *ECS::Entity(childEntityId).GetTransform() = childTransform;

            if (childObjectData.basePrefabId != aPrefab.GetPrefabId())
            {
                Wonder::PrefabAsset& otherPrefab = Wonder::GetPrefabRegistry().AccessAssetFromID(childObjectData.basePrefabId);
                const std::vector<Wonder::ComponentData>& compoents = otherPrefab.AccessPrefabObjects().at(otherPrefab.GetHeadObjectId()).components;
                
                for (const Wonder::ComponentData& componentData : compoents)
                {
                    componentData.GetType()->OnGameComponentCreate(ECS::Entity(childEntityId), componentData.AccessData());
                }
                
                if (otherPrefab.GetPrefabObjects().size() > 1)
                {
                    InstantiateChildren(otherPrefab,childEntityId,childTransform,childEntityId,childObjectId);
                }
            }
            else
            {
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

        ECS::CreateEntity(isStatic, layer,aHeadEntityId,aPrefab.GetHeadObjectId()).GetID();
        
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


