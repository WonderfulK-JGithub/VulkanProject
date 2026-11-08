#include "TextureDescriptor.h"

#include "Engine.h"
#include "Sampler.h"
#include "Util/Logging/Logging.h"

namespace
{
    static Vulkan::TextureDescriptor* localInstance;
}

Vulkan::TextureDescriptor& Vulkan::TextureDescriptor::GetInstance()
{
    return *localInstance;
}

void Vulkan::TextureDescriptor::Init()
{
    localInstance = this;
    
    std::array<vk::DescriptorBindingFlags,2> bindingFlags =
        {vk::DescriptorBindingFlagBits::ePartiallyBound,vk::DescriptorBindingFlagBits::ePartiallyBound};
    
    
    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
    bindingFlagsInfo.pBindingFlags = bindingFlags.data();
    bindingFlagsInfo.bindingCount = 2;
    
    std::array<vk::DescriptorSetLayoutBinding,2> layoutBindings{};
    layoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    layoutBindings[0].descriptorCount = MAX_TEXTURES;
    layoutBindings[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    layoutBindings[0].binding = 0;
    
    layoutBindings[1].stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    layoutBindings[1].descriptorCount = MAX_CUBE_MAPS;
    layoutBindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    layoutBindings[1].binding = 1;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.flags        = vk::DescriptorSetLayoutCreateFlags();
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings    = layoutBindings.data();
    layoutInfo.pNext        = &bindingFlagsInfo;
    
    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
    
    try
    {
        myDescriptorSetLayout = logicalDevice.createDescriptorSetLayout(layoutInfo);
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create DescriptorSetLayout, %s", err.what());
    }
    
    vk::DescriptorSetAllocateInfo allocateInfo;

    allocateInfo.descriptorPool = Engine::GetInstance().GetTextureDescriptorPool();
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &myDescriptorSetLayout;
    
    myDescriptorSet = logicalDevice.allocateDescriptorSets(allocateInfo)[0];
}

void Vulkan::TextureDescriptor::Destroy()
{
    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
    logicalDevice.destroyDescriptorSetLayout(myDescriptorSetLayout);
}

vk::DescriptorSet Vulkan::TextureDescriptor::GetDescriptorSet()
{
    return myDescriptorSet;
}

vk::DescriptorSetLayout Vulkan::TextureDescriptor::GetDescriptorSetLayout()
{
    return myDescriptorSetLayout;
}

void Vulkan::TextureDescriptor::AddTexture(const Wonder::TextureId aId, const vk::ImageView aView,bool aIsCubemap)
{
    if (aIsCubemap)
    {
        myCubemapIdToView.insert(aId,aView);
    }
    else
    {
        myIdToView.insert(aId,aView);
    }
    myNeedsDescriptorUpdate = true;
}

void Vulkan::TextureDescriptor::RemoveTexture(const Wonder::TextureId aId,bool aIsCubemap)
{
    if (aIsCubemap)
    {
        myCubemapIdToView.erase(aId);
    }
    else
    {
        myIdToView.erase(aId);
    }
    
    myNeedsDescriptorUpdate = true;
}

uint32_t Vulkan::TextureDescriptor::GetTextureIndex(const Wonder::TextureId aId)
{
    const auto it = myIdToIndex.find(aId);
    
    if (it != myIdToIndex.end())
    {
        return it->second;
    }
    
    return 0;
}

uint32_t Vulkan::TextureDescriptor::GetCubemapIndex(Wonder::TextureId aId)
{
    const auto it = myCubemapIdToIndex.find(aId);
    
    if (it != myCubemapIdToIndex.end())
    {
        return it->second;
    }
    
    return 0;
}

void Vulkan::TextureDescriptor::AddRenderImage(RenderImageId aId, vk::ImageView aView)
{
    myRenderImageIdToView.insert(aId,aView);
    myNeedsDescriptorUpdate = true;
}

void Vulkan::TextureDescriptor::RemoveRenderImage(RenderImageId aId)
{
    myRenderImageIdToView.erase(aId);
    myNeedsDescriptorUpdate = true;
}

uint32_t Vulkan::TextureDescriptor::GetRenderImageTextureIndex(RenderImageId aId)
{
    const auto it = myRenderImageIdToIndex.find(aId);
    
    if (it != myRenderImageIdToIndex.end())
    {
        return it->second;
    }
    
    return 0;
}

bool Vulkan::TextureDescriptor::NeedsDescriptorUpdate() const
{
    return myNeedsDescriptorUpdate;
}

void Vulkan::TextureDescriptor::UpdateDescriptorSet()
{
    myIdToIndex.clear();
    myCubemapIdToIndex.clear();
    myRenderImageIdToIndex.clear();
    
    std::vector<vk::DescriptorImageInfo> imageInfos;
    uint64_t descriptorCount = myIdToView.size() + myRenderImageIdToView.size();
    imageInfos.resize(descriptorCount);

    const vk::Sampler sampler = Vulkan::GetSampler(Vulkan::SamplerType::BilinearRepeat);
    
    uint32_t i = 0;
    for (; i < myIdToView.size(); ++i)
    {
        myIdToIndex.insert(std::make_pair(myIdToView.dataKeys()[i],i));
        
        imageInfos[i].imageView = myIdToView.data()[i];
        imageInfos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfos[i].sampler = sampler;
    }
    
    for (uint32_t j = 0; j < myRenderImageIdToView.size(); ++j)
    {
        uint32_t index = i + j;
        myRenderImageIdToIndex.insert(std::make_pair(myRenderImageIdToView.dataKeys()[j],index));
        
        imageInfos[index].imageView = myRenderImageIdToView.data()[j];
        imageInfos[index].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfos[index].sampler = sampler;
    }
    
    std::vector<vk::DescriptorImageInfo> cubemapImageInfos;
    cubemapImageInfos.resize(myCubemapIdToView.size());
    
    for (uint32_t j = 0; j < myCubemapIdToView.size(); ++j)
    {
        myCubemapIdToIndex.insert(std::make_pair(myCubemapIdToView.dataKeys()[j],j));
        
        cubemapImageInfos[j].imageView = myCubemapIdToView.data()[j];
        cubemapImageInfos[j].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        cubemapImageInfos[j].sampler = sampler;
    }
    
    std::array<vk::WriteDescriptorSet,2> descriptorWrites;
    descriptorWrites[0].dstSet          = myDescriptorSet;
    descriptorWrites[0].dstBinding      = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites[0].descriptorCount = static_cast<uint32_t>(imageInfos.size());
    descriptorWrites[0].pImageInfo      = imageInfos.data();
    
    descriptorWrites[1].dstSet          = myDescriptorSet;
    descriptorWrites[1].dstBinding      = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites[1].descriptorCount = static_cast<uint32_t>(cubemapImageInfos.size());
    descriptorWrites[1].pImageInfo      = cubemapImageInfos.data();
    
    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
    logicalDevice.updateDescriptorSets(descriptorWrites, {});
    
    myNeedsDescriptorUpdate = false;
}
