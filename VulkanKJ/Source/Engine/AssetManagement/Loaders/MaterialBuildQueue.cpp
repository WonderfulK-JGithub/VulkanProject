#include "MaterialBuildQueue.h"

#include <vector>

#include "AssetManagement/AssetRegistry.h"

namespace MaterialBuildQueue
{
    using namespace Wonder;
    static std::vector<MaterialId> localQueuedMaterials;
    void AddMaterialToQueue(MaterialId aMaterialId)
    {
        localQueuedMaterials.push_back(aMaterialId);
    }

    bool NeedsRebuild()
    {
        return localQueuedMaterials.empty() == false;
    }

    void RebuildQueuedMaterials()
    {
        auto& materials = GetMaterialRegistry().AccessAssetMap();
        for (auto& materialId : localQueuedMaterials)
        {
            materials.at(materialId).Build();
        }
        localQueuedMaterials.clear();
    }
}


