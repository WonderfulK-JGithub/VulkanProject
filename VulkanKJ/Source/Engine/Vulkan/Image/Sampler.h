#pragma once
#include <vulkan/vulkan.hpp>

namespace Vulkan
{
    enum class SamplerType : uint32_t
    {
        BilinearRepeat,
        Count
    };
    
    void CreateSamplers();
    void DestroySamplers();
    
    vk::Sampler GetSampler(SamplerType aSamplerType);
}
