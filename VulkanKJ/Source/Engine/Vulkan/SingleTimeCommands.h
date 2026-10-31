#pragma once
#include <vulkan/vulkan.hpp>

namespace Vulkan
{
	void StartSingleTimeCommand(vk::CommandBuffer aCommandBuffer);

	void EndSingleTimeCommand(vk::CommandBuffer aCommandBuffer,vk::Queue aQueue);
}