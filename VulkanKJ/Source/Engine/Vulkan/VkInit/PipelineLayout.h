#pragma once
#include <vulkan/vulkan.hpp>

#include "Descriptors.h"
#include "Vulkan/RenderStructs.h"

namespace Vulkan
{
    enum class PipelineLayoutType
    {
        DefaultPipeline,
        Invalid,
    };
    
    void CreatePipelineLayouts();
    void DestroyPipelineLayouts();
    
    vk::PipelineLayout GetPipelineLayout(PipelineLayoutType aPipelineLayoutType);
    
    struct DescriptorSetCollection
    {
        VkInit::VulkanDescriptorSet globalDescriptorSet;
        VkInit::VulkanDescriptorSet modelInstanceDescriptorSet;
    };
	
    void BindPipelineLayout(vk::CommandBuffer aCommandBuffer, const DescriptorSetCollection& aDescriptorSets, PipelineLayoutType aType);
}
