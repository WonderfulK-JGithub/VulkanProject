#include "Framebuffer.h"
#include "RenderPass.h"
#include "Util/Logging/Logging.h"

void VkInit::CreateFrameBuffer(const FrameBufferInput& aInput, vk::Framebuffer& aFrameBuffer, const std::vector<vk::ImageView>& aAttachments,
	Vulkan::RenderStageType aRenderStage)
{
	vk::FramebufferCreateInfo framebufferInfo{};
	framebufferInfo.flags           = vk::FramebufferCreateFlags();
	framebufferInfo.renderPass      = VkInit::GetRenderPass(aRenderStage);
	framebufferInfo.attachmentCount = static_cast<uint32_t>(aAttachments.size());
	framebufferInfo.pAttachments    = aAttachments.data();
	framebufferInfo.width           = aInput.frameExtent.width;
	framebufferInfo.height          = aInput.frameExtent.height;
	framebufferInfo.layers          = 1;

	try
	{
		aFrameBuffer = aInput.device.createFramebuffer(framebufferInfo);
		LOG_DEBUG("%s","Created frame buffer");
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to create framebuffer, %s.", err.what());
	}
}
