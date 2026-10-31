#include "Device.h"
#include <iostream>
#include "../VulkanConstants.h"
#include <set>
#include "../DeviceLogging.h"
#include <algorithm>

#include "Util/Logging/Logging.h"

bool CheckDeviceExtensionSupport(
	const vk::PhysicalDevice& aDevice,
	const std::vector<const char*>& aRequestedExtensions
) {

	std::set<std::string> requiredExtensions(aRequestedExtensions.begin(), aRequestedExtensions.end());

	LOG_DEBUG("%s","Checking device extensions");

	for (vk::ExtensionProperties& extension : aDevice.enumerateDeviceExtensionProperties()) {

		LOG_DEBUG("\t\"%s",extension.extensionName);

		//remove this from the list of required extensions (set checks for equality automatically)
		requiredExtensions.erase(extension.extensionName);
	}

	//if the set is empty then all requirements have been satisfied
	return requiredExtensions.empty();
}

bool IsSuitable(const vk::PhysicalDevice& device) {
	LOG_DEBUG("%s","Checking if device is suitable");
	
	/*
	* A device is suitable if it can present to the screen, ie support
	* the swapchain extension
	*/
	const std::vector<const char*> requestedExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	LOG_DEBUG("We are requesting device extensions:");
	for (const char* extension : requestedExtensions) {
		LOG_DEBUG("\t\"%s",extension);
	}

	if (CheckDeviceExtensionSupport(device, requestedExtensions)) {

		LOG_DEBUG("%s","Device can support the requested extensions!");
	}
	else {

		LOG_DEBUG("%s","Device does not support the requested extensions!");

		return false;
	}
	return true;
}


vk::PhysicalDevice VkInit::ChoosePhysicalDevice(vk::Instance& aInstance)
{
	LOG_INFO("%s","Choosing Physical Device:\n");

    std::vector<vk::PhysicalDevice> availableDevices = aInstance.enumeratePhysicalDevices();

	LOG_INFO("System physical device count: %s",std::to_string(availableDevices.size()).c_str());

	for (const vk::PhysicalDevice& device : availableDevices)
	{
		LogDeviceProperties(device);

		if (IsSuitable(device))
		{
			return device;
		}
	}
	
    return nullptr;
}

VkInit::QueueFamilyIndices VkInit::FindQueueFamilies(vk::PhysicalDevice& aDevice, vk::SurfaceKHR aSurface)
{
	QueueFamilyIndices indices;

	std::vector<vk::QueueFamilyProperties> queueFamilies = aDevice.getQueueFamilyProperties();

	LOG_DEBUG("System queue family support count: %s",std::to_string(queueFamilies.size()).c_str());

	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		vk::QueueFamilyProperties& queueFamily = queueFamilies[i];
		
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
			
			LOG_DEBUG("Queue Family %d was chosen for graphics",i);
		}

		if (aDevice.getSurfaceSupportKHR(i, aSurface))
		{
			indices.presentFamily = i;

			LOG_DEBUG("Queue Family %d was chosen for presenting",i);
		}

		if (indices.IsComplete())
		{
			break;
		}
	}

	return indices;
}

vk::Device VkInit::CreateLogicalDevice(vk::PhysicalDevice& aPhysicaDevice, vk::SurfaceKHR aSurface)
{
	QueueFamilyIndices indices = FindQueueFamilies(aPhysicaDevice, aSurface);

	std::vector<uint32_t> uniqueIndices;
	uniqueIndices.push_back(indices.graphicsFamily.value());
	if (indices.graphicsFamily.value() != indices.presentFamily.value())
	{
		uniqueIndices.push_back(indices.presentFamily.value());
	}

	float queuePriority = 1.f;

	std::vector< vk::DeviceQueueCreateInfo> queueInfos;

	for(auto index : uniqueIndices)
	{
		queueInfos.push_back(vk::DeviceQueueCreateInfo{
		vk::DeviceQueueCreateFlags(),
		index,
		1,
		&queuePriority
			});
	}

	std::vector<const char*> deviceExtensions{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	

	vk::PhysicalDeviceDescriptorIndexingFeatures deviceDescriptorIndexingFeatures{};
	deviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
	deviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
	deviceDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
	
	
	
	vk::PhysicalDeviceFeatures deviceFeatures{};

	vk::DeviceCreateInfo deviceCreateInfo{
		vk::DeviceCreateFlags(),
		static_cast<uint32_t>(queueInfos.size()),
		queueInfos.data(),
		0,//(uint32_t)enabledLayers.size(),
		nullptr,//enabledLayers.data(),
		static_cast<uint32_t>(deviceExtensions.size()),
		deviceExtensions.data(),
		&deviceFeatures,
		&deviceDescriptorIndexingFeatures
	};

	try
	{
		vk::Device device = aPhysicaDevice.createDevice(deviceCreateInfo);

		LOG_INFO("Device creation succeeded, GPU has been successfully abstracted");
		return device;
	}
	catch (vk::SystemError aError)
	{
		LOG_ERROR("Device creation failed, %s", aError.what());
	}

	return nullptr;
}

vk::Queue VkInit::GetGraphicsQueue(vk::PhysicalDevice& aPhysicaDevice, vk::Device aDevice, vk::SurfaceKHR aSurface)
{
	QueueFamilyIndices indices = FindQueueFamilies(aPhysicaDevice, aSurface);

	return aDevice.getQueue(indices.graphicsFamily.value(), 0);
}

vk::Queue VkInit::GetPresentQueue(vk::PhysicalDevice& aPhysicaDevice, vk::Device aDevice, vk::SurfaceKHR aSurface)
{
	QueueFamilyIndices indices = FindQueueFamilies(aPhysicaDevice, aSurface);

	return aDevice.getQueue(indices.presentFamily.value(), 0);
}



