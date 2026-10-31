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
    
    vk::DescriptorBindingFlags bindingFlags =
        vk::DescriptorBindingFlagBits::ePartiallyBound;
    
    
    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
    bindingFlagsInfo.pBindingFlags = &bindingFlags;
    bindingFlagsInfo.bindingCount = 1;
    
    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
    layoutBinding.descriptorCount = 100;
    layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    layoutBinding.binding = 0;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.flags = vk::DescriptorSetLayoutCreateFlags();
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;
    layoutInfo.pNext = &bindingFlagsInfo;
    
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

void Vulkan::TextureDescriptor::AddTexture(const Wonder::TextureId aId, const vk::ImageView aView)
{
    myIdToView.insert(aId,aView);
    myNeedsDescriptorUpdate = true;
}

void Vulkan::TextureDescriptor::RemoveTexture(const Wonder::TextureId aId)
{
    myIdToView.erase(aId);
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
    
    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet          = myDescriptorSet;
    descriptorWrite.dstBinding      = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
    descriptorWrite.pImageInfo      = imageInfos.data();
    
    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
    logicalDevice.updateDescriptorSets(descriptorWrite, {});
    
    myNeedsDescriptorUpdate = false;
}
