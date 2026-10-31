#include "ResolutionData.h"

#include <vector>

#include "AssetManagement/AssetRegistry.h"

namespace 
{
    std::vector<Vulkan::ResolutionId> localAvailableResolutions;
    Vulkan::ResolutionId localResolutionIdCounter = 1;
    
    Goose::MappedVector<Vulkan::ResolutionId,vk::Extent2D> localCurrentResolutions;
}

Vulkan::ResolutionId Vulkan::GetOrCreateResolution()
{
    ResolutionId id{};
    if (localAvailableResolutions.empty())
    {
        id = ++localResolutionIdCounter;
    }
    else
    {
        id = localAvailableResolutions.back();
        localAvailableResolutions.pop_back();
    }
    return id;
}

void Vulkan::SetResolution(const ResolutionId aId, const vk::Extent2D aResolution)
{
    localCurrentResolutions.insert_or_assign(aId, aResolution);

    auto& materials = Wonder::GetMaterialRegistry().AccessAssetMap();
    for (uint32_t i = 0; i < materials.size(); ++i)
    {
        materials.data()[i].BuildOrRebuildGraphicsPipeline(aId,aResolution);
    }
}

void Vulkan::DiscardResolution(const ResolutionId aId)
{
    localAvailableResolutions.push_back(aId);
    localCurrentResolutions.erase(aId);
    
    auto& materials = Wonder::GetMaterialRegistry().AccessAssetMap();
    for (uint32_t i = 0; i < materials.size(); ++i)
    {
        materials.data()[i].DestroyGraphicsPipeline(aId);
    }
}

const Goose::MappedVector<Vulkan::ResolutionId, vk::Extent2D>& Vulkan::GetAllResolutions()
{
    return localCurrentResolutions;
}
