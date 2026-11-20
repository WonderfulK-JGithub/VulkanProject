#include "PrefabAsset.h"

#include "AssetRegistry.h"
#include "ECS/Reflection/ComponentReflectionUtil.h"
#include "ECS/Reflection/ComponentType.h"
#include "ID/IdFunctions.h"
#include "Property/PropertySaveAndLoadFunctions.h"

void Wonder::PrefabAsset::Init(const nlohmann::json& aJson)
{
    myId = PrefabId(aJson[ID_JSON_KEY]);
    
    myHeadObjectId.id = aJson["myHeadObject"];
    
    const nlohmann::json& objectDataListJson = aJson["objectDataList"];
    
    for (auto& objectDataJson : objectDataListJson)
    {
        PrefabObjectData data{};
        LoadProperty(data.localTransform.translation, objectDataJson, "translation");
        LoadProperty(data.localTransform.rotation, objectDataJson, "rotation");
        LoadProperty(data.localTransform.scale, objectDataJson, "scale");
        data.objectId.id = objectDataJson["objectId"];
        data.parentId.id = objectDataJson["parentId"];
        data.basePrefabId.id = objectDataJson["basePrefabId"];
        data.name = objectDataJson["name"];
        
        LoadComponentsFromJson(data.components, const_cast<nlohmann::json&>(objectDataJson["components"]));
        
        myPrefabObjects.emplace(data.objectId,data);
        
        if (data.parentId != NULL_PREFAB_OBJECT)
        {
            if (myParentToChildrenMap.contains(data.parentId))
            {
                myParentToChildrenMap.at(data.parentId).push_back(data.objectId);
            }
            else
            {
                myParentToChildrenMap.emplace(data.parentId,std::vector<PrefabObjectId>{data.objectId});
            }
        }
        
        if (data.basePrefabId != myId && std::ranges::find(myDependencies,data.basePrefabId) == myDependencies.end())
        {
            myDependencies.push_back(data.basePrefabId);
        }
        
        if (objectDataJson.contains("overrides"))
        {
            auto& overrides = myOverrides[data.objectId];
            LoadOverridesFromJson(overrides, objectDataJson["overrides"]);
        }
    }
    
    
}

void Wonder::PrefabAsset::SaveToJson(nlohmann::json& aJson,const char* aName)
{
    aJson[ID_JSON_KEY] = myId.id;
    aJson[NAME_JSON_KEY] = aName == nullptr ? GetPrefabRegistry().GetNameFromID(myId) : aName;
    
    aJson["myHeadObject"] = myHeadObjectId.id;
    
    aJson["objectDataList"] = nlohmann::json::array();
    nlohmann::json& objectDataListJson = aJson["objectDataList"];
    
    for (uint32_t i = 0; i < myPrefabObjects.size(); ++i)
    {
        nlohmann::json objectDataJson;
        
        PrefabObjectData& data = myPrefabObjects.data()[i];
        SaveProperty(data.localTransform.translation, objectDataJson, "translation");
        SaveProperty(data.localTransform.rotation, objectDataJson, "rotation");
        SaveProperty(data.localTransform.scale, objectDataJson, "scale");
        objectDataJson["objectId"] = data.objectId.id;
        objectDataJson["parentId"] = data.parentId.id;
        objectDataJson["basePrefabId"] = data.basePrefabId.id;
        objectDataJson["name"] = data.name.c_str();
        
        SaveComponentsToJson(data.components, objectDataJson["components"]);
        
        objectDataListJson.emplace_back(objectDataJson);
        
        if (myOverrides.contains(data.objectId))
        {
            auto& overrides = myOverrides[data.objectId];
            LoadOverridesFromJson(overrides, objectDataJson["overrides"]);
        }
    }
}

void Wonder::PrefabAsset::AdoptPrefab(PrefabObjectId aParent, PrefabId aId)
{
    PrefabAsset& adoptedPrefab = GetPrefabRegistry().AccessAssetFromID(aId);
    
    PrefabObjectData& copyData = adoptedPrefab.myPrefabObjects.at(adoptedPrefab.GetHeadObjectId());
    PrefabObjectData newData{};
    
    newData.name = copyData.name + "(Adopted)";
    newData.components = copyData.components;
    newData.basePrefabId = aId;
    newData.parentId = aParent;
    newData.objectId = GeneratePrefabObjectId();
    
    myPrefabObjects.emplace(newData.objectId,newData);
    if (myParentToChildrenMap.contains(newData.parentId))
    {
        myParentToChildrenMap.at(newData.parentId).push_back(newData.objectId);
    }
    else
    {
        myParentToChildrenMap.emplace(newData.parentId,std::vector<PrefabObjectId>{newData.objectId});
    }

    if (std::ranges::find(myDependencies,aId) == myDependencies.end())
    {
        myDependencies.push_back(aId);
    }
}

bool Wonder::PrefabAsset::ContainsPrefab(PrefabId aPrefabId) const
{
    for (auto& id : myDependencies)
    {
        if (id == aPrefabId)
        {
            return true;
        }
    }
    for (auto& id : myDependencies)
    {
        auto& prefab = GetPrefabRegistry().AccessAssetFromID(id);
        if (prefab.ContainsPrefab(aPrefabId))
        {
            return true;
        }
    }
    
    return false;
}

Wonder::PrefabAsset Wonder::PrefabAsset::MakeDefault(const char* aName)
{
    PrefabAsset asset{};
    
    asset.myPrefabObjects.clear();
    asset.myParentToChildrenMap.clear();
    
    asset.myId = GeneratePrefabId();
    
    PrefabObjectData headObject{};
    headObject.objectId = GeneratePrefabObjectId();
    headObject.basePrefabId = asset.myId;
    headObject.name = aName;
    
    asset.myPrefabObjects.emplace(headObject.objectId,headObject);
    asset.myHeadObjectId = headObject.objectId;
    
    return asset;
}

Goose::MappedVector<Wonder::PrefabObjectId, Wonder::PrefabObjectData>& Wonder::PrefabAsset::AccessPrefabObjects()
{
    return myPrefabObjects;
}

const Goose::MappedVector<Wonder::PrefabObjectId, Wonder::PrefabObjectData>& Wonder::PrefabAsset::GetPrefabObjects() const
{
    return myPrefabObjects;
}

std::unordered_map<Wonder::PrefabObjectId,std::vector<Wonder::PrefabObjectId>>& Wonder::PrefabAsset::AccessParentToChildrenMap()
{
    return myParentToChildrenMap;
}

const std::unordered_map<Wonder::PrefabObjectId,std::vector<Wonder::PrefabObjectId>>& Wonder::PrefabAsset::GetParentToChildrenMap() const
{
    return myParentToChildrenMap;
}

std::unordered_map<Wonder::PrefabObjectId, Wonder::ComponentOverrides>& Wonder::PrefabAsset::AccessOverrides()
{
    return myOverrides;
}

const std::unordered_map<Wonder::PrefabObjectId, Wonder::ComponentOverrides>& Wonder::PrefabAsset::GetOverrides() const
{
    return myOverrides;
}

Wonder::PrefabObjectData& Wonder::PrefabAsset::AccessHeadObjectData()
{
    return myPrefabObjects.at(myHeadObjectId);
}

const Wonder::PrefabObjectData& Wonder::PrefabAsset::GetHeadObjectData() const
{
    return myPrefabObjects.at(myHeadObjectId);
}

Wonder::PrefabObjectId Wonder::PrefabAsset::GetHeadObjectId() const
{
    return myHeadObjectId;
}

Wonder::PrefabId Wonder::PrefabAsset::GetPrefabId() const
{
    return myId;
}

std::vector<Wonder::ComponentData>& Wonder::PrefabAsset::AccessComponents(PrefabObjectId aPrefabObjectId)
{
    auto& prefabObject = myPrefabObjects.at(aPrefabObjectId);
    
    if (prefabObject.basePrefabId == myId)
    {
        return prefabObject.components;
    }
    else
    {
        auto& prefab = GetPrefabRegistry().AccessAssetFromID(prefabObject.basePrefabId);
        return prefab.AccessComponents(prefab.GetHeadObjectId());
    }
}
