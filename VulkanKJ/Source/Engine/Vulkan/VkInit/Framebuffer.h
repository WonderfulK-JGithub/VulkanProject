#pragma once
#include <vulkan/vulkan.hpp>
#include "Swapchain.h"
#include <vector>
#include "Pipeline.h"

namespace VkInit
{
	struct FrameBufferInput
	{
		vk::Device device;
		vk::Extent2D frameExtent;
	};

	
	void CreateFrameBuffer(const FrameBufferInput& aInput,vk::Framebuffer& aFrameBuffer, const std::vector<vk::ImageView>& aAttachments,
		Vulkan::RenderStageType aRenderStage);
	
}