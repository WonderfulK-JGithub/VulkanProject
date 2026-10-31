#include "Swapchain.h"
#include "Device.h"
#include "../DeviceLogging.h"
#include <algorithm>

#include "Vulkan/Image/Image.h"
#include "Util/Logging/Logging.h"

vk::SurfaceFormatKHR VkInit::ChooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& aFormats)
{
	constexpr vk::Format preferedFormat = vk::Format::eB8G8R8A8Unorm;

	for (vk::SurfaceFormatKHR format : aFormats)
	{
		if (format.format == preferedFormat)
		{
			return preferedFormat;
		}
	}

	return aFormats.front();
}

vk::PresentModeKHR VkInit::ChooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& aPresentModes)
{
	constexpr vk::PresentModeKHR preferedPresentMode = vk::PresentModeKHR::eMailbox;

	for (vk::PresentModeKHR presentMode : aPresentModes)
	{
		if (presentMode == preferedPresentMode)
		{
			return preferedPresentMode;
		}
	}

	return aPresentModes.front();
}

vk::Extent2D VkInit::ChooseSwapchainExtent(uint32_t aWidth, uint32_t aHeight, vk::SurfaceCapabilitiesKHR aCapabilities)
{
	if (aCapabilities.currentExtent.width != UINT32_MAX) {
		return aCapabilities.currentExtent;
	}

	vk::Extent2D extent = { aWidth, aHeight };

	extent.width = std::min(
		aCapabilities.maxImageExtent.width,
		std::max(aCapabilities.minImageExtent.width, extent.width)
	);

	extent.height = std::min(
		aCapabilities.maxImageExtent.height,
		std::max(aCapabilities.minImageExtent.height, extent.height)
	);

	return extent;
}

VkInit::SwapchainSupportDetails VkInit::GetSwapchainSupport(const vk::PhysicalDevice& aDevice, const vk::SurfaceKHR& aSurface)
{
	SwapchainSupportDetails support;

	support.capabilities = aDevice.getSurfaceCapabilitiesKHR(aSurface);

#ifndef _RETAIL
	LOG_DEBUG("%s", "Swapchain can support the following surface capabilities:");

	LOG_DEBUG("\tmin image count: %d",support.capabilities.minImageCount);
	LOG_DEBUG("\tmax image count: %ds",support.capabilities.maxImageCount);

	LOG_DEBUG("\twidth: %d",support.capabilities.currentExtent.width);
	LOG_DEBUG("\theight: %d",support.capabilities.currentExtent.height);
	

	LOG_DEBUG("%s","\tMinimum support extent:");
	LOG_DEBUG("\t\twidth: %d",support.capabilities.minImageExtent.width);
	LOG_DEBUG("\t\theight: %d",support.capabilities.minImageExtent.height);

	LOG_DEBUG("%s","\tmaximum support extent:");
	LOG_DEBUG("\t\twidth: %d",support.capabilities.maxImageExtent.width);
	LOG_DEBUG("\t\theight: %d",support.capabilities.maxImageExtent.height);

	LOG_DEBUG("\tmaximum image array layers: %d", support.capabilities.maxImageArrayLayers);
	
	LOG_DEBUG("%s","\tsupported transforms:");
	LogTransformBits(support.capabilities.supportedTransforms);

	LOG_DEBUG("%s","\tcurrent transforms:");
	LogTransformBits(support.capabilities.currentTransform);

	LogAlphaCompositeBits(support.capabilities.supportedCompositeAlpha);

	LogImageUsageBits(support.capabilities.supportedUsageFlags);
	
#endif
	

	support.formats = aDevice.getSurfaceFormatsKHR(aSurface);

	for (vk::SurfaceFormatKHR supportedFormat : support.formats)
	{
		LOG_DEBUG("supported pixel format: %s",vk::to_string(supportedFormat.format).c_str());
		LOG_DEBUG("supported color space: %s",vk::to_string(supportedFormat.colorSpace).c_str());
	}

	support.presentModes = aDevice.getSurfacePresentModesKHR(aSurface);

	for (auto presentMode : support.presentModes)
	{
		LogPresentMode(presentMode);
	}

	return support;
}

VkInit::SwapchainInfo VkInit::CreateSwapchain(vk::Device aLogicalDevice, vk::PhysicalDevice aPhysicalDevice, const vk::SurfaceKHR& aSurface, int aWidth, int aHeight)
{
	SwapchainSupportDetails support = GetSwapchainSupport(aPhysicalDevice, aSurface);

	vk::SurfaceFormatKHR format = ChooseSwapchainSurfaceFormat(support.formats);
	vk::PresentModeKHR presentMode = ChooseSwapchainPresentMode(support.presentModes);
	vk::Extent2D extent = ChooseSwapchainExtent(aWidth, aHeight, support.capabilities);

	uint32_t imageCount = std::min(support.capabilities.maxImageCount, support.capabilities.minImageCount + 1);

	vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR
	{
		vk::SwapchainCreateFlagsKHR(),
		aSurface,
		imageCount,
		format.format,
		format.colorSpace,
		extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment
	};

	QueueFamilyIndices indices = FindQueueFamilies(aPhysicalDevice, aSurface);

	if (indices.graphicsFamily.value() != indices.presentFamily.value())
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		uint32_t queueFamiltIndices[]{ indices.graphicsFamily.value() ,indices.presentFamily.value() };
		createInfo.pQueueFamilyIndices = queueFamiltIndices;
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	createInfo.preTransform = support.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

	SwapchainInfo bundle{};
	bundle.minImages = support.capabilities.minImageCount;

	try
	{
		bundle.swapchain = aLogicalDevice.createSwapchainKHR(createInfo);
	}
	catch (vk::SystemError err)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	bundle.images = aLogicalDevice.getSwapchainImagesKHR(bundle.swapchain);

	bundle.frames.resize(bundle.images.size());

	bundle.format = format.format;
	bundle.depthFormat = Vulkan::FindSupportedFormat(aPhysicalDevice, { vk::Format::eD32Sfloat,vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	bundle.extent = extent;

	return bundle;
}

void VkInit::SwapchainFrame::Destroy()
{
	logicalDevice.destroyFence(inFlightFence);
	logicalDevice.destroySemaphore(imageAvailable);
	logicalDevice.destroySemaphore(renderFinnished);
}
