#pragma once
#include "Swapchain.h"
#include "Device.h"
#include <vector>

namespace VkInit
{
	struct CommandBufferInput
	{
		vk::Device device;
		vk::CommandPool commandPool;
		std::vector<VkInit::SwapchainFrame>& frames;
	};

	vk::CommandPool MakeCommandPool(vk::Device aDevice,vk::PhysicalDevice aPhysicalDevice,vk::SurfaceKHR aSurface);

	vk::CommandBuffer MakeCommandBuffer(vk::Device aDevice, vk::CommandPool aCommandPool);
	void MakeFrameCommandBuffers(const CommandBufferInput& aInput);
}