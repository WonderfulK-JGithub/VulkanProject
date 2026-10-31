#pragma once
#include "Id/IdTypes.h"

namespace MaterialBuildQueue
{
    void AddMaterialToQueue(Wonder::MaterialId aMaterialId);
    bool NeedsRebuild();
    void RebuildQueuedMaterials();
}
