#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <optional>

namespace VkInit
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	vk::PhysicalDevice ChoosePhysicalDevice(vk::Instance& aInstance);

	QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice& aDevice,vk::SurfaceKHR aSurface);

	vk::Device CreateLogicalDevice(vk::PhysicalDevice& aPhysicaDevice, vk::SurfaceKHR aSurface);

	vk::Queue GetGraphicsQueue(vk::PhysicalDevice& aPhysicaDevice, vk::Device aDevice, vk::SurfaceKHR aSurface);

	vk::Queue GetPresentQueue(vk::PhysicalDevice& aPhysicaDevice, vk::Device aDevice, vk::SurfaceKHR aSurface);

	

	
}