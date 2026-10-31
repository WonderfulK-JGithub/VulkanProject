#pragma once

#include <vulkan/vulkan.hpp>

namespace Vulkan
{
	struct ImageInput
	{
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		int width;
		int height;
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
		vk::Format format;
		uint32_t arrayCount;
		vk::ImageCreateFlags flags;
	};

	struct ImageLayoutTransitionInput
	{
		vk::CommandBuffer commandBuffer;
		vk::Queue queue;
		vk::Image image;

		vk::ImageLayout oldLayout;
		vk::ImageLayout newLayout;
		uint32_t arrayCount;;
	};

	struct BufferImageCopyInput
	{
		vk::CommandBuffer commandBuffer;
		vk::Queue queue;
		vk::Buffer srcBuffer;
		vk::Image dstImage;
		int width;
		int height;
		uint32_t arrayCount;
	};

	

	vk::Image MakeImage(const ImageInput& aInput);
	vk::DeviceMemory MakeImageMemory(const ImageInput& aInput, vk::Image aImage);

	void TransitionImageLayoutSingleTime(const ImageLayoutTransitionInput& aInput);
	void TransitionImageLayout(vk::CommandBuffer aCommandBuffer,vk::Image aImage,vk::ImageLayout aOldLayout, vk::ImageLayout aNewLayout,vk::ImageAspectFlags aAspectMask,uint32_t aArrayCount);
	void CopyBufferToImage(const BufferImageCopyInput& aInput);

	vk::ImageView MakeImageView(vk::Device aLogicalDevice,vk::Image aImage,vk::Format aFormat, vk::ImageAspectFlagBits aAspect,
		vk::ImageViewType aViewType, uint32_t aArrayCount);

	vk::Format FindSupportedFormat(vk::PhysicalDevice aPhysicalDevice, const std::vector<vk::Format>& aCandidates, vk::ImageTiling aTiling, vk::FormatFeatureFlags aFeatures);
}