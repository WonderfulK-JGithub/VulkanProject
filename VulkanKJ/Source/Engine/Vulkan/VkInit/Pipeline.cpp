#include "Pipeline.h"
#include <vector>
#include "Shaders/Shaders.h"
#include "../VkMesh.h"
#include "RenderPass.h"
#include "Util/Logging/Logging.h"

vk::Pipeline VkInit::MakeGraphicsPipeline(const GraphicsPipelineInBundle& aSpecification)
{
    using namespace Internal;
    
    vk::GraphicsPipelineCreateInfo pipelineInfo;

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = Vulkan::GetVertexInputInfo(aSpecification.vertexType);
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = aSpecification.vertexType == Vulkan::VertexLayoutType::Line ? 
        MakeLineInputAssemblyInfo() : MakeDefaultInputAssemblyInfo();
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

    vk::Viewport viewPort = MakeViewport(aSpecification);
    
    vk::Rect2D scissor = MakeScissor(aSpecification);

    vk::PipelineViewportStateCreateInfo viewPortStateInfo = MakeViewportStateInfo(viewPort,scissor);
    pipelineInfo.pViewportState = &viewPortStateInfo;

    vk::PipelineRasterizationStateCreateInfo rasterizerInfo = MakeRasterizerInfo();
    pipelineInfo.pRasterizationState = &rasterizerInfo;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    shaderStages.push_back(Vulkan::GetShaderInfo(aSpecification.vertexShaderfilePath,aSpecification.device, vk::ShaderStageFlagBits::eVertex));
    shaderStages.push_back(Vulkan::GetShaderInfo(aSpecification.fragmentShaderfilePath, aSpecification.device, vk::ShaderStageFlagBits::eFragment));

    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages    = shaderStages.data();

    vk::PipelineDepthStencilStateCreateInfo depthInfo{};
    depthInfo.flags = vk::PipelineDepthStencilStateCreateFlags();
    depthInfo.depthTestEnable = true;
    depthInfo.depthWriteEnable = true;
    depthInfo.depthCompareOp = vk::CompareOp::eLess;
    depthInfo.depthBoundsTestEnable = false;
    depthInfo.stencilTestEnable = false;
    pipelineInfo.pDepthStencilState = &depthInfo;

    vk::PipelineMultisampleStateCreateInfo multisamplingInfo = MakeMultisampleInfo();
    pipelineInfo.pMultisampleState = &multisamplingInfo;

    vk::PipelineColorBlendAttachmentState colorBlendAttachementState{};
    colorBlendAttachementState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachementState.blendEnable = VK_FALSE;
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo = MakeColorBlendStateInfo(colorBlendAttachementState);
    pipelineInfo.pColorBlendState = &colorBlendInfo;

    
    LOG_DEBUG("%s","Creating PipelineLayout");
    
    pipelineInfo.layout = aSpecification.layout;

    pipelineInfo.renderPass = GetRenderPass(aSpecification.stage);

    pipelineInfo.basePipelineHandle = nullptr;
    
    LOG_DEBUG("%s","Creating Graphics Pipeline");
    
    vk::Pipeline graphicsPipeline{};
    try
    {
        graphicsPipeline = aSpecification.device.createGraphicsPipeline(nullptr, pipelineInfo).value;
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create Graphics Pipeline, %s", err.what());
    }
    
    return graphicsPipeline;
}

namespace VkInit::Internal
{
    vk::PipelineInputAssemblyStateCreateInfo MakeDefaultInputAssemblyInfo()
    {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
        inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

        return inputAssemblyInfo;
    }

    vk::PipelineInputAssemblyStateCreateInfo MakeLineInputAssemblyInfo()
    {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
        inputAssemblyInfo.topology = vk::PrimitiveTopology::eLineList;

        return inputAssemblyInfo;
    }

    vk::Viewport MakeViewport(const GraphicsPipelineInBundle& aSpecification)
    {
        vk::Viewport viewPort{};
        viewPort.x        = 0.f;
        viewPort.y        = 0.f;
        viewPort.width    = static_cast<float>(aSpecification.renderSize.width);
        viewPort.height   = static_cast<float>(aSpecification.renderSize.height);
        viewPort.minDepth = 0.f;
        viewPort.maxDepth = 1.f;

        return viewPort;
    }

    vk::Rect2D MakeScissor(const GraphicsPipelineInBundle& aSpecification)
    {
        vk::Rect2D scissor{};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = aSpecification.renderSize;

        return scissor;
    }

    vk::PipelineViewportStateCreateInfo MakeViewportStateInfo(const vk::Viewport& aViewport, const vk::Rect2D& aScissor)
    {
        vk::PipelineViewportStateCreateInfo viewPortStateInfo{};
        viewPortStateInfo.flags = vk::PipelineViewportStateCreateFlags();
        viewPortStateInfo.viewportCount = 1;
        viewPortStateInfo.pViewports = &aViewport;
        viewPortStateInfo.scissorCount = 1;
        viewPortStateInfo.pScissors = &aScissor;

        return viewPortStateInfo;
    }

    vk::PipelineRasterizationStateCreateInfo MakeRasterizerInfo()
    {
        vk::PipelineRasterizationStateCreateInfo rasterizerInfo{};
        rasterizerInfo.flags = vk::PipelineRasterizationStateCreateFlags();
        rasterizerInfo.depthClampEnable = VK_FALSE;
        rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
        rasterizerInfo.lineWidth = 1.f;
        rasterizerInfo.cullMode = vk::CullModeFlagBits::eNone;
        rasterizerInfo.frontFace = vk::FrontFace::eClockwise;
        rasterizerInfo.depthBiasEnable = VK_FALSE;

        return rasterizerInfo;
    }



    vk::PipelineMultisampleStateCreateInfo MakeMultisampleInfo()
    {
        vk::PipelineMultisampleStateCreateInfo multisamplingInfo{};
        multisamplingInfo.flags = vk::PipelineMultisampleStateCreateFlags();
        multisamplingInfo.sampleShadingEnable = VK_FALSE;
        multisamplingInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

        return multisamplingInfo;
    }

    vk::PipelineColorBlendStateCreateInfo MakeColorBlendStateInfo(const vk::PipelineColorBlendAttachmentState& aColorBlendAttachment)
    {
        vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
        colorBlendInfo.flags = vk::PipelineColorBlendStateCreateFlags();
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = vk::LogicOp::eCopy;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &aColorBlendAttachment;
        colorBlendInfo.blendConstants[0] = 0.f;
        colorBlendInfo.blendConstants[1] = 0.f;
        colorBlendInfo.blendConstants[2] = 0.f;
        colorBlendInfo.blendConstants[3] = 0.f;

        return colorBlendInfo;
    }
}


