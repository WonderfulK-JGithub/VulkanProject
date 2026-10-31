#pragma once
#include <vulkan/vulkan.hpp>
#include "../RenderStructs.h"

namespace VkInit
{
	void CreateRenderPasses(vk::Device aDevice, vk::Format aSwapchainImageFormat, vk::Format aSwapchainDepthFormat);
	void DestroyRenderPasses(vk::Device aDevice);

	vk::RenderPass GetRenderPass(Vulkan::RenderStageType aRenderStage);
}