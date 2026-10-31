#pragma once
#include <vulkan/vulkan.hpp>

namespace Vulkan
{
	struct BufferCreationInput
	{
		size_t size;
		vk::BufferUsageFlags usage;
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		vk::MemoryPropertyFlags memortyProperties;
	};

	struct Buffer
	{
		vk::Buffer buffer;
		vk::DeviceMemory bufferMemory;
	};

	Buffer CreateBuffer(const BufferCreationInput& aInput);

	uint32_t FindMemoryTypeIndex(vk::PhysicalDevice aPhysicalDevice, uint32_t aSupportedMemoryIndices, vk::MemoryPropertyFlags aRequestedProperties);

	void AllocateBufferMemory(Buffer& aBuffer, const BufferCreationInput& aInput);

	void CopyBuffer(Buffer& aSource, Buffer& aDestination, vk::DeviceSize aSize, vk::Queue aQueue, vk::CommandBuffer aCommandBuffer);
}