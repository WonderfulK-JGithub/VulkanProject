#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "Util/Container/MappedVector.hpp"

namespace Vulkan
{
    using ResolutionId = uint32_t;
    
    constexpr ResolutionId SWAPCHAIN_RESOLUTION_ID = 69420;
    constexpr ResolutionId INVALID_RESOLUTION_ID = 0;
    
    ResolutionId GetOrCreateResolution();
    void SetResolution(ResolutionId aId,vk::Extent2D aResolution);
    void DiscardResolution(ResolutionId aId);
    
    const Goose::MappedVector<Vulkan::ResolutionId,vk::Extent2D>& GetAllResolutions();
}
