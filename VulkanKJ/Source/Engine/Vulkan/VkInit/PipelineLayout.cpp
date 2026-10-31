#include "PipelineLayout.h"

#include "Engine.h"
#include "Util/Logging/Logging.h"

namespace Vulkan
{
    static std::unordered_map<RenderStageType, vk::PipelineLayout> localPipelineLayouts;
    
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
        
        localPipelineLayouts[RenderStageType::Cubemap] = MakePipelineLayout(logicalDevice,
        { VkInit::GetStageDescriptorSetLayout(RenderStageType::Cubemap),
        TextureDescriptor::GetInstance().GetDescriptorSetLayout()});
        
        localPipelineLayouts[RenderStageType::Forward] = MakePipelineLayout(logicalDevice,
        { VkInit::GetStageDescriptorSetLayout(RenderStageType::Forward),
        TextureDescriptor::GetInstance().GetDescriptorSetLayout()});
        
        localPipelineLayouts[RenderStageType::FullscreenOffscreen] = MakePipelineLayout(logicalDevice,
        { VkInit::GetStageDescriptorSetLayout(RenderStageType::FullscreenOffscreen),
        TextureDescriptor::GetInstance().GetDescriptorSetLayout()});
        
        localPipelineLayouts[RenderStageType::FullscreenSwapchain] = MakePipelineLayout(logicalDevice,
        { VkInit::GetStageDescriptorSetLayout(RenderStageType::FullscreenSwapchain),
        TextureDescriptor::GetInstance().GetDescriptorSetLayout()});
    }

    void DestroyPipelineLayouts()
    {
        vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
        
        for (auto& pipelineLayout : localPipelineLayouts | std::views::values)
        {
            logicalDevice.destroyPipelineLayout(pipelineLayout);
        }
    }

    vk::PipelineLayout GetPipelineLayout(RenderStageType aRenderStageType)
    {
        return localPipelineLayouts[aRenderStageType];
    }
}


