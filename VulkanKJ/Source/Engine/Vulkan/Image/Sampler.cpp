#include "Sampler.h"

#include "Engine.h"
#include "Util/Logging/Logging.h"

namespace Vulkan
{
    static std::vector<VkSampler> localSamplers;
}

void Vulkan::CreateSamplers()
{
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.flags = vk::SamplerCreateFlags();
    samplerInfo.minFilter = vk::Filter::eNearest;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = false;
    samplerInfo.maxAnisotropy = 1.f;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = false;
    samplerInfo.compareEnable = false;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.f;
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = 0.f;

    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
    
    localSamplers.resize(static_cast<uint32_t>(SamplerType::Count));
    try
    {
        localSamplers[static_cast<uint32_t>(SamplerType::BilinearRepeat)] = logicalDevice.createSampler(samplerInfo);
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create sampler, %s", err.what());
    }
}

void Vulkan::DestroySamplers()
{
    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
    for (auto& sampler : localSamplers)
    {
        logicalDevice.destroySampler(sampler);
    }
}

vk::Sampler Vulkan::GetSampler(SamplerType aSamplerType)
{
    return localSamplers[static_cast<uint32_t>(aSamplerType)];
}
