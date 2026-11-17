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

    auto loadComponentDatas = [](std::vector<ComponentData>& aComponentDatas, nlohmann::json& aJson)
    {
        static std::string typeName;
        for (auto& componentDataJson : aJson)
        {
            typeName                        = componentDataJson["!type"];
            const ComponentTypeBase* type = GetComponentType(Tga::StringRegistry::RegisterOrGetString(typeName));

            if (type == nullptr)
            {
                continue;
            }

            aComponentDatas.emplace_back(type);
            Reflection::globalReflectionMode = Reflection::ReflectionMode::Load;
            Reflection::globalJsonSaveData = &componentDataJson;
            type->ReflectComponent(aComponentDatas.back().AccessData());
            aComponentDatas.back().AccessOverridesFlag() = componentDataJson["allowedOverrides"];
        }
    };
    
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
        
        loadComponentDatas(data.components, const_cast<nlohmann::json&>(objectDataJson["components"]));
        
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
    }
}

void Wonder::PrefabAsset::SaveToJson(nlohmann::json& aJson,const char* aName)
{
    aJson[ID_JSON_KEY] = myId.id;
    aJson[NAME_JSON_KEY] = aName == nullptr ? GetPrefabRegistry().GetNameFromID(myId) : aName;
    
    aJson["myHeadObject"] = myHeadObjectId.id;

    auto saveComponentDatas = [&](std::vector<ComponentData>& aComponentDatas, nlohmann::json& aJson)
    {
        for (ComponentData& componentData : aComponentDatas)
        {
            const ComponentTypeBase* type = componentData.GetType();

            nlohmann::json currentSetJson;
            currentSetJson["!type"]            = type->GetName(); // ! in order to make it come first in the file XD
            currentSetJson["allowedOverrides"] = componentData.GetOverridesFlag();

            Reflection::globalReflectionMode = Reflection::ReflectionMode::Save;
            Reflection::globalJsonSaveData = &currentSetJson;
            type->ReflectComponent(componentData.AccessData());
            aJson.emplace_back(currentSetJson);
        }
    };
    
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
        
        saveComponentDatas(data.components, objectDataJson["components"]);
        
        objectDataListJson.emplace_back(objectDataJson);
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

Wonder::PrefabObjectId Wonder::PrefabAsset::GetHeadObjectId() const
{
    return myHeadObjectId;
}

Wonder::PrefabId Wonder::PrefabAsset::GetPrefabId() const
{
    return myId;
}
