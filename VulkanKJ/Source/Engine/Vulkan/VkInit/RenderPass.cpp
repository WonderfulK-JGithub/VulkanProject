#include "RenderPass.h"
#include <unordered_map>
#include <iostream>

#include "Util/Logging/Logging.h"

namespace VkInit
{
	std::unordered_map<Vulkan::RenderStageType, vk::RenderPass> renderPasses;

    vk::AttachmentDescription MakeColorAttachment(vk::Format aSwapchainImageFormat, vk::AttachmentLoadOp aLoadOp, 
        vk::ImageLayout aInitialLayout,vk::ImageLayout aFinalLayout)
    {
        vk::AttachmentDescription colorAttachment{};
        colorAttachment.flags = vk::AttachmentDescriptionFlags();
        colorAttachment.format = aSwapchainImageFormat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = aLoadOp;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = aInitialLayout;
        colorAttachment.finalLayout = aFinalLayout;

        return colorAttachment;
    }

    vk::AttachmentDescription MakeDepthAttachment(vk::Format aDepthFormat)
    {
        vk::AttachmentDescription depthAttachment{};
        depthAttachment.flags = vk::AttachmentDescriptionFlags();
        depthAttachment.format = aDepthFormat;
        depthAttachment.samples = vk::SampleCountFlagBits::e1;
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        return depthAttachment;
    }

    vk::RenderPass CreateRenderPass(vk::Device aDevice,const std::vector<vk::AttachmentDescription>& aAttachments, bool aUseDepth)
    {
        std::vector< vk::AttachmentReference> attachmentReferences;

        size_t colorAttachmentCount = aUseDepth ? aAttachments.size() - 1 : aAttachments.size();

        for (size_t i = 0; i < aAttachments.size(); i++)
        {
            vk::AttachmentReference attachmentRef{};
            attachmentRef.attachment = static_cast<uint32_t>(i);
            if (aUseDepth && i == aAttachments.size() - 1) 
            {
                attachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            }
            else
            {
                attachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
            }
            attachmentReferences.push_back(attachmentRef);
        }

        vk::SubpassDescription subpass{};
        subpass.flags                   = vk::SubpassDescriptionFlags();
        subpass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount    = static_cast<uint32_t>(colorAttachmentCount);
        subpass.pColorAttachments       = &attachmentReferences[0];
        subpass.pDepthStencilAttachment = aUseDepth ? &attachmentReferences[1] : nullptr;

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.flags           = vk::RenderPassCreateFlags();
        renderPassInfo.attachmentCount = static_cast<uint32_t>(aAttachments.size());
        renderPassInfo.pAttachments    = aAttachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;

        try
        {
            return aDevice.createRenderPass(renderPassInfo);
        }
        catch (vk::SystemError err)
        {
            LOG_ERROR("Failed to create Render Pass, %s", err.what());
        }

        return {};
    }


	void CreateRenderPasses(vk::Device aDevice, vk::Format aSwapchainImageFormat, vk::Format aSwapchainDepthFormat)
	{
        {
            std::vector< vk::AttachmentDescription> attachments;
            attachments.push_back(MakeColorAttachment(aSwapchainImageFormat, vk::AttachmentLoadOp::eClear, 
                vk::ImageLayout::eUndefined,vk::ImageLayout::eColorAttachmentOptimal));
            
            renderPasses.insert({ Vulkan::RenderStageType::Cubemap,CreateRenderPass(aDevice,attachments,false) });
        }

        {
            std::vector< vk::AttachmentDescription> attachments;
            attachments.push_back(MakeColorAttachment(aSwapchainImageFormat, vk::AttachmentLoadOp::eDontCare, 
                vk::ImageLayout::eUndefined,vk::ImageLayout::eColorAttachmentOptimal));
            attachments.push_back(MakeDepthAttachment(aSwapchainDepthFormat));

            renderPasses.insert({ Vulkan::RenderStageType::Forward,CreateRenderPass(aDevice,attachments,true) });
        }
        
        {
            std::vector< vk::AttachmentDescription> attachments;
            attachments.push_back(MakeColorAttachment(aSwapchainImageFormat, vk::AttachmentLoadOp::eClear, 
                vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR));
            
            renderPasses.insert({ Vulkan::RenderStageType::FullscreenSwapchain,CreateRenderPass(aDevice,attachments,false) });
        }
        
        {
            std::vector< vk::AttachmentDescription> attachments;
            attachments.push_back(MakeColorAttachment(aSwapchainImageFormat, vk::AttachmentLoadOp::eClear, 
                vk::ImageLayout::eUndefined,vk::ImageLayout::eColorAttachmentOptimal));
            
            renderPasses.insert({ Vulkan::RenderStageType::FullscreenOffscreen,CreateRenderPass(aDevice,attachments,false) });
        }
        
        {
            std::vector< vk::AttachmentDescription> attachments;
            attachments.push_back(MakeColorAttachment(aSwapchainImageFormat, vk::AttachmentLoadOp::eDontCare, 
                vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR));
            
            renderPasses.insert({ Vulkan::RenderStageType::ImGui,CreateRenderPass(aDevice,attachments,false) });
        }
	}
    void DestroyRenderPasses(vk::Device aDevice)
    {
        for (auto& [key, renderPass] : renderPasses)
        {
            aDevice.destroyRenderPass(renderPass);
        }
    }
    vk::RenderPass GetRenderPass(Vulkan::RenderStageType aRenderStage)
    {
        return renderPasses.at(aRenderStage);
    }
}


