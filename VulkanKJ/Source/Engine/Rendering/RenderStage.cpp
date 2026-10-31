#include "RenderStage.h"

#include "Engine.h"
#include "Vulkan/VkInit/Framebuffer.h"

void RenderStage::AddRenderImage(const RenderImage& aRenderImage)
{
    myRenderImages.push_back(&aRenderImage);
}

void RenderStage::CreateFrameBuffers(Vulkan::RenderStageType aType,vk::Extent2D aFrameExtent)
{
    auto& engine = Engine::GetInstance();
    VkInit::FrameBufferInput frameInput;
    frameInput.device = engine.GetLogicalDevice();
    frameInput.frameExtent = aFrameExtent;
    
    
    myFrameBuffers.resize(engine.GetSwapchainFrames().size());
    for (uint32_t i = 0; i < engine.GetSwapchainFrames().size(); ++i)
    {
        std::vector<vk::ImageView> attachments;
        for (auto& renderImage : myRenderImages)
        {
            attachments.push_back(renderImage->GetImageView(i));
        }

        VkInit::CreateFrameBuffer(frameInput,myFrameBuffers[i],attachments,aType);
    }
}

void RenderStage::Destroy()
{
    vk::Device logicalDevice = Engine::GetInstance().GetLogicalDevice();
    for (auto& frameBuffer : myFrameBuffers)
    {
        logicalDevice.destroyFramebuffer(frameBuffer);
    }
    
    myFrameBuffers.clear();
    myRenderImages.clear();
}

void RenderStage::StartRenderPass(vk::CommandBuffer aCommandBuffer, vk::RenderPass aRenderPass,vk::Extent2D aRenderExtent) const
{
    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = aRenderPass;
    renderPassInfo.framebuffer = myFrameBuffers[Engine::GetInstance().GetCurrentSwapchainIndex()];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = aRenderExtent;

    std::vector<vk::ClearValue> clearValues;
    for (auto& renderImage : myRenderImages)
    {
        if (renderImage->IsDepthImage())
        {
            vk::ClearValue clearDepth{};
            clearDepth.depthStencil = vk::ClearDepthStencilValue({ 1.f,0 });
            clearValues.push_back(clearDepth);
        }
        else
        {
            vk::ClearValue clearColor = { std::array<float,4>{1.f,0.5f,0.25f,1.f} };
            clearValues.push_back(clearColor);
        }
    }
    
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    aCommandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}
