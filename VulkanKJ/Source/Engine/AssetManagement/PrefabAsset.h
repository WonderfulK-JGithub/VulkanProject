#pragma once
#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>

#include "ECS/Components/TransformComponent.h"
#include "ECS/Reflection/ComponentData.h"
#include "ID/IdTypes.h"
#include "Util/Container/FixedString.h"
#include "Util/Container/MappedVector.hpp"

namespace Wonder
{
    struct PrefabObjectData
    {
        PrefabObjectId objectId{};
        PrefabObjectId parentId{};
        PrefabId basePrefabId;
        Goose::TransformTRS localTransform;
        std::vector<ComponentData> components;
        Goose::String32 name = "123";
    };
    
    
    
    class PrefabAsset
    {
    public:
        void Init(const nlohmann::json& aJson);
        void SaveToJson(nlohmann::json& aJso, const char* aName);
        
        void AdoptPrefab(PrefabObjectId aParent,PrefabId aId);
        bool ContainsPrefab(PrefabId aPrefabId) const;
    
        static PrefabAsset MakeDefault(const char* aName);
        
        Goose::MappedVector<PrefabObjectId,PrefabObjectData>& AccessPrefabObjects();
        const Goose::MappedVector<PrefabObjectId,PrefabObjectData>& GetPrefabObjects() const;
        
        std::unordered_map<PrefabObjectId,std::vector<PrefabObjectId>>& AccessParentToChildrenMap();
        const std::unordered_map<PrefabObjectId,std::vector<PrefabObjectId>>& GetParentToChildrenMap() const;
        
        std::unordered_map<PrefabObjectId,ComponentOverrides>& AccessOverrides();
        const std::unordered_map<PrefabObjectId,ComponentOverrides>& GetOverrides() const;
        
        PrefabObjectData& AccessHeadObjectData();
        const PrefabObjectData& GetHeadObjectData() const;
        
        PrefabObjectId GetHeadObjectId() const;
        PrefabId GetPrefabId() const;
        
        std::vector<ComponentData>& AccessComponents(PrefabObjectId aPrefabObjectId);
    private:
        
        Goose::MappedVector<PrefabObjectId,PrefabObjectData> myPrefabObjects;
        std::unordered_map<PrefabObjectId,std::vector<PrefabObjectId>> myParentToChildrenMap;
        
        std::unordered_map<PrefabObjectId,ComponentOverrides> myOverrides;
        
        std::vector<PrefabId> myDependencies;
        
        PrefabObjectId myHeadObjectId{};
        PrefabId myId{};
    };
}


