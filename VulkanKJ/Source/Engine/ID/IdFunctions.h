#pragma once
#include "IdTypes.h"

namespace Wonder
{
    TextureId GenerateTextureId();
    ModelId GenerateModelId();
    MaterialId GenerateMaterialId();
    PrefabId GeneratePrefabId();
    
    bool IdCountersNeedSaving();
    void SaveIdCounters();
    void LoadIdCounters();
}
