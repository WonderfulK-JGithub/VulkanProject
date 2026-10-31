#include "Image.h"
#include "../VkMemory.h"
#include "../VkInit/Descriptors.h"
#include "../SingleTimeCommands.h"
#include "Util/Logging/Logging.h"

vk::Image Vulkan::MakeImage(const ImageInput& aInput)
{
	vk::ImageCreateInfo imageInfo;
	imageInfo.flags = aInput.flags;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(aInput.width, aInput.height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = aInput.arrayCount;
	imageInfo.format = aInput.format;
	imageInfo.tiling = aInput.tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = aInput.usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.samples = vk::SampleCountFlagBits::e1;

	try
	{
		return aInput.logicalDevice.createImage(imageInfo);
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to create image, %s",err.what());
	}

	return{};
}

vk::DeviceMemory Vulkan::MakeImageMemory(const ImageInput& aInput, const vk::Image aImage)
{
	vk::MemoryRequirements requirements = aInput.logicalDevice.getImageMemoryRequirements(aImage);

	vk::MemoryAllocateInfo allocateInfo;
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = Vulkan::FindMemoryTypeIndex(aInput.physicalDevice, requirements.memoryTypeBits, aInput.memoryProperties);

	try
	{
		vk::DeviceMemory imageMemory = aInput.logicalDevice.allocateMemory(allocateInfo);
		aInput.logicalDevice.bindImageMemory(aImage, imageMemory, 0);
		return imageMemory;
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to allocate image memory, %s",err.what());
	}

	return vk::DeviceMemory();
}

void Vulkan::TransitionImageLayoutSingleTime(const ImageLayoutTransitionInput& aInput)
{
	Vulkan::StartSingleTimeCommand(aInput.commandBuffer);

	TransitionImageLayout(aInput.commandBuffer,aInput.image,aInput.oldLayout,aInput.newLayout,vk::ImageAspectFlagBits::eColor,aInput.arrayCount);

	Vulkan::EndSingleTimeCommand(aInput.commandBuffer, aInput.queue);
}

void Vulkan::TransitionImageLayout(const vk::CommandBuffer aCommandBuffer, const vk::Image aImage, const vk::ImageLayout aOldLayout, 
	const vk::ImageLayout aNewLayout,vk::ImageAspectFlags aAspectMask, const uint32_t aArrayCount)
{
	vk::ImageSubresourceRange access{};
	access.aspectMask = aAspectMask;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = aArrayCount;

	vk::ImageMemoryBarrier barrier{};
	barrier.oldLayout = aOldLayout;
	barrier.newLayout = aNewLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = aImage;
	barrier.subresourceRange = access;

	// vk::PipelineStageFlags srcStage;
	// vk::PipelineStageFlags dstStage;

	auto LayoutToStageMaskPair = [](vk::ImageLayout aLayout)
	{
		switch (aLayout)
		{
			case vk::ImageLayout::eUndefined:
			default:
				return std::make_pair(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTopOfPipe),vk::AccessFlagBits::eNoneKHR);
			case vk::ImageLayout::eTransferSrcOptimal:
				return std::make_pair(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),vk::AccessFlagBits::eTransferWrite);
			case vk::ImageLayout::eShaderReadOnlyOptimal:
				return std::make_pair(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eFragmentShader),vk::AccessFlagBits::eShaderRead);
			case vk::ImageLayout::eColorAttachmentOptimal:
				return std::make_pair(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput),vk::AccessFlagBits::eColorAttachmentWrite);
			case vk::ImageLayout::eDepthAttachmentOptimal:
				return std::make_pair(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
					,vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		}
	};
	
	auto [srcStage,srcMask] = LayoutToStageMaskPair(aOldLayout);
	auto [dstStage,dstMask] = LayoutToStageMaskPair(aNewLayout);
	
	barrier.srcAccessMask = srcMask;
	barrier.dstAccessMask = dstMask;

	aCommandBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), {}, {}, barrier);
}

void Vulkan::CopyBufferToImage(const BufferImageCopyInput& aInput)
{
	Vulkan::StartSingleTimeCommand(aInput.commandBuffer);

	vk::BufferImageCopy copy{};
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;

	vk::ImageSubresourceLayers access{};
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.mipLevel = 0;
	access.baseArrayLayer = 0;
	access.layerCount = aInput.arrayCount;
	copy.imageSubresource = access;

	copy.imageOffset = vk::Offset3D();
	copy.imageExtent = vk::Extent3D(aInput.width, aInput.height);
	copy.imageExtent.depth = 1;

	aInput.commandBuffer.copyBufferToImage(aInput.srcBuffer, aInput.dstImage, vk::ImageLayout::eTransferDstOptimal, copy);

	Vulkan::EndSingleTimeCommand(aInput.commandBuffer, aInput.queue);
}

vk::ImageView Vulkan::MakeImageView(const vk::Device aLogicalDevice, const vk::Image aImage, const vk::Format aFormat, const vk::ImageAspectFlagBits aAspect, const vk::ImageViewType aViewType, const uint32_t aArrayCount)
{
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = aImage;
	viewInfo.viewType = aViewType;
	viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
	viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
	viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
	viewInfo.components.a = vk::ComponentSwizzle::eIdentity;

	viewInfo.subresourceRange.aspectMask = aAspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = aArrayCount;

	viewInfo.format = aFormat;

	return aLogicalDevice.createImageView(viewInfo);
}

vk::Format Vulkan::FindSupportedFormat(const vk::PhysicalDevice aPhysicalDevice, const std::vector<vk::Format>& aCandidates, const vk::ImageTiling aTiling, const vk::FormatFeatureFlags aFeatures)
{
	for (const vk::Format& format : aCandidates)
	{
		vk::FormatProperties properties = aPhysicalDevice.getFormatProperties(format);

		if (aTiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & aFeatures) == aFeatures)
		{
			return format;
		}

		if (aTiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & aFeatures) == aFeatures)
		{
			return format;
		}
	}

	return vk::Format();
}
