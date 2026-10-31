#pragma once
#include <vulkan/vulkan.hpp>

namespace VkInit
{
	vk::Semaphore CreateSemaphore(vk::Device aDevice);

	vk::Fence CreateFence(vk::Device aDevice);
}