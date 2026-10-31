#pragma once
#include <vulkan/vulkan.hpp>

namespace VkInit
{
	vk::DebugUtilsMessengerEXT MakeDebugMessenger(vk::Instance& aInstance, vk::detail::DispatchLoaderDynamic& aDLD);

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	void LogDeviceProperties(const vk::PhysicalDevice& aDevice);

	void LogTransformBits(vk::SurfaceTransformFlagsKHR bits);
	void LogAlphaCompositeBits(vk::CompositeAlphaFlagsKHR bits);
	void LogImageUsageBits(vk::ImageUsageFlags bits);
	void LogPresentMode(vk::PresentModeKHR presentMode);
}