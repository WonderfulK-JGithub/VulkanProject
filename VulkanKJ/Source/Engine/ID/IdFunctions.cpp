#include "IdFunctions.h"

#include <nlohmann/json.hpp>

#include "Util/FileManagement/FileFunctions.h"
#include "Util/FileManagement/FileTypes.h"
#include "Util/Logging/Logging.h"

namespace Wonder
{
    static bool localIdCountersNeedSaving{};
    
    static uint64_t localTextureIdCounter{1};
    static uint64_t localModelIdCounter{1};
    static uint64_t localMaterialIdCounter{1};
    static uint64_t localPrefabIdCounter{1};
}

Wonder::TextureId Wonder::GenerateTextureId()
{
    Wonder::TextureId newId{localTextureIdCounter};
    localTextureIdCounter++;
    localIdCountersNeedSaving = true;
    return newId;
}

Wonder::ModelId Wonder::GenerateModelId()
{
    Wonder::ModelId newId{localModelIdCounter};
    localModelIdCounter++;
    localIdCountersNeedSaving = true;
    return newId;
}

Wonder::MaterialId Wonder::GenerateMaterialId()
{
    Wonder::MaterialId newId{localMaterialIdCounter};
    localMaterialIdCounter++;
    localIdCountersNeedSaving = true;
    return newId;
}

Wonder::PrefabId Wonder::GeneratePrefabId()
{
    Wonder::PrefabId newId{localPrefabIdCounter};
    localPrefabIdCounter++;
    localIdCountersNeedSaving = true;
    return newId;
}

bool Wonder::IdCountersNeedSaving()
{
    return localIdCountersNeedSaving;
}

void Wonder::SaveIdCounters()
{
    const Filepath idCountersFilepath = FileFunctions::GetAppdataPath() / "IdCounters.json";
    
    const nlohmann::json idCountersJson{
        {"localTextureIdCounter",localTextureIdCounter},
        {"localModelIdCounter",localModelIdCounter},
        {"localMaterialIdCounter",localMaterialIdCounter},
        {"localPrefabIdCounter",localPrefabIdCounter}
    };
    
    FileFunctions::SaveFileFromJson(idCountersFilepath, idCountersJson);
    
    localIdCountersNeedSaving = false;
    LOG_DEBUG("%s","Id counters saved");
}

void Wonder::LoadIdCounters()
{
    const Filepath idCountersFilepath = FileFunctions::GetAppdataPath() / "IdCounters.json";
    
    if (exists(idCountersFilepath) == false)
    {
        return;
    }
    
    nlohmann::json idCountersJson;
    FileFunctions::LoadFileToJson(idCountersFilepath, idCountersJson);
    
#define SAFE_JSON_ASSIGN(JSON,NAME,TO_ASSIGN)if (JSON.contains(NAME))\
    {\
        TO_ASSIGN = JSON[NAME];\
    }
    
    localTextureIdCounter = idCountersJson["localTextureIdCounter"];
    localModelIdCounter = idCountersJson["localModelIdCounter"];
    localMaterialIdCounter = idCountersJson["localMaterialIdCounter"];
    SAFE_JSON_ASSIGN(idCountersJson, "localPrefabIdCounter",localPrefabIdCounter)
}
