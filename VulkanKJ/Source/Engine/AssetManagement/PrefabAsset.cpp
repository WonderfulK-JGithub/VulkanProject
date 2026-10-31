#include "PrefabAsset.h"

#include "AssetRegistry.h"

void Wonder::PrefabAsset::Init(const nlohmann::json& aJson)
{
    myId = PrefabId(aJson[ID_JSON_KEY]);
}

void Wonder::PrefabAsset::SaveToJson(nlohmann::json& aJson)
{
    aJson[ID_JSON_KEY] = {myId.id};
    aJson[NAME_JSON_KEY] = GetPrefabRegistry().GetNameFromID(myId);
}

std::vector<Wonder::ComponentData>& Wonder::PrefabAsset::AccessComponents()
{
    return myComponents;
}
