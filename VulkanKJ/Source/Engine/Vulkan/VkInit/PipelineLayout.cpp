#include "PipelineLayout.h"

#include "Engine.h"
#include "Util/Logging/Logging.h"

namespace Vulkan
{
    static std::unordered_map<PipelineLayoutType, vk::PipelineLayout> localPipelineLayouts;
    
    vk::PipelineLayout MakePipelineLayout(vk::Device aDevice, const std::vector<vk::DescriptorSetLayout>& aDescriptorSetLayouts)
    {
        vk::PipelineLayoutCreateInfo layoutInfo;
        layoutInfo.flags          = vk::PipelineLayoutCreateFlags();
        layoutInfo.setLayoutCount = static_cast<uint32_t>(aDescriptorSetLayouts.size());
        layoutInfo.pSetLayouts    = aDescriptorSetLayouts.data();
    
        layoutInfo.pushConstantRangeCount = 1;
        vk::PushConstantRange pushConstantInfo;
        pushConstantInfo.offset = 0;
        pushConstantInfo.size = 128;
        pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eFragment;
        layoutInfo.pPushConstantRanges = &pushConstantInfo;

        try
        {
            return aDevice.createPipelineLayout(layoutInfo);
        }
        catch(vk::SystemError err)
        {
            LOG_ERROR("Failed to create pipeline layout, %s.", err.what());
        }

        return {};
    }
    
    void CreatePipelineLayouts()
    {
        vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
        
        localPipelineLayouts[PipelineLayoutType::DefaultPipeline] = MakePipelineLayout(logicalDevice,
        { 
            TextureDescriptor::GetInstance().GetDescriptorSetLayout(),
            VkInit::GetDescriptorSetLayout(VkInit::DescriptorLayoutType::Camera),
            VkInit::GetDescriptorSetLayout(VkInit::DescriptorLayoutType::ModelInstance),
        });
    }

    void DestroyPipelineLayouts()
    {
        vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
        
        for (auto& pipelineLayout : localPipelineLayouts | std::views::values)
        {
            logicalDevice.destroyPipelineLayout(pipelineLayout);
        }
    }

    vk::PipelineLayout GetPipelineLayout(PipelineLayoutType aType)
    {
        return localPipelineLayouts[aType];
    }

    void BindPipelineLayout(vk::CommandBuffer aCommandBuffer, const DescriptorSetCollection& aDescriptorSets, PipelineLayoutType aType)
    {
        switch (aType)
        {
            case PipelineLayoutType::DefaultPipeline:
            {
                vk::PipelineLayout pipelineLayout = Vulkan::GetPipelineLayout(Vulkan::PipelineLayoutType::DefaultPipeline);
                
                aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 
                    Vulkan::TextureDescriptor::GetInstance().GetDescriptorSet(),{});
                aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, 
                aDescriptorSets.globalDescriptorSet.GetDescriptorSet(),{});
                aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 2, 
                aDescriptorSets.modelInstanceDescriptorSet.GetDescriptorSet(),{});
                
            }
                break;
        }
    }
}


