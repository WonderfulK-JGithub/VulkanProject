#include "Instance.h"

#include <GLFW/glfw3.h>

#include "enginepch.h"
#include "../VulkanConstants.h"
#include "Util/Logging/Logging.h"

static uint32_t localInstanceVersion;

vk::Instance VkInit::MakeInstance(const char* aApplicationName)
{
	uint32_t version{};
	vkEnumerateInstanceVersion(&version);

	LOG_INFO("System can support Vulkan\nVariant: %d\nMajor: %d\nMinor: %d\nPatch: %d",
		VK_API_VERSION_VARIANT(version),VK_API_VERSION_MAJOR(version),VK_API_VERSION_MINOR(version),VK_API_VERSION_PATCH(version));
	
	version = VK_MAKE_API_VERSION(0, 1, 3, 0);

	localInstanceVersion = version;

	vk::ApplicationInfo appInfo{};
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = aApplicationName;
	appInfo.applicationVersion = version;
	appInfo.engineVersion = version;
	appInfo.apiVersion = version;
	appInfo.pEngineName = "Wonderful Engine";

	uint32_t glfwExtensionCount{};
	const char** glfwExtensions{};

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	
	
	std::vector<const char*> extensions;
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		extensions.push_back(glfwExtensions[i]);
	}

#ifdef USE_VALIDATION_LAYER
	extensions.push_back("VK_EXT_debug_utils");
	
	LOG_DEBUG("%s","Extensions Required by GLFW and Instance:");
	for (const char* extension : extensions)
	{
		LOG_DEBUG("\t%s", extension);
	}
#endif

	std::vector<const char*> layers;

#ifdef USE_VALIDATION_LAYER
	layers.push_back(VK_VALIDATION_LAYER);
#endif

	if (IsExtensionsSupported(extensions,layers) == false)
	{
		return nullptr;
	}
	
	

	vk::InstanceCreateInfo createInfo{}; 
	
	createInfo.pNext = nullptr;
	createInfo.flags = vk::InstanceCreateFlags();
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	
	try
	{
		return vk::createInstance(createInfo);
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to create instance, %s", err.what());
	}

	return nullptr;
}

uint32_t VkInit::GetInstanceVersion()
{
	return localInstanceVersion;
}

bool VkInit::IsExtensionsSupported(const std::vector<const char*>& aExtensions, const std::vector<const char*>& aLayers)
{
	std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

	LOG_DEBUG("%s","Device support following extensions:");
	for (const vk::ExtensionProperties& supportedExtension: supportedExtensions)
	{
		LOG_DEBUG("\t%s", supportedExtension.extensionName);
	}

	for (const char* extension : aExtensions)
	{
		bool found = false;
		for (const vk::ExtensionProperties& supportedExtension : supportedExtensions)
		{
			if (strcmp(extension, supportedExtension.extensionName) == 0)
			{
				found = true;
				break;
			}
		}

		if (found == false)
		{
			LOG_WARNING("Extension %s is not supported", extension);
			return false;
		}
	}

	std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

	LOG_DEBUG("%s","Device support following layers:");
	for (const vk::LayerProperties& supportedLayer : supportedLayers)
	{
		LOG_DEBUG("\t%s", supportedLayer.layerName);
	}

	for (const char* layer : aLayers)
	{
		bool found = false;
		for (const vk::LayerProperties& supportedLayer : supportedLayers)
		{
			if (strcmp(layer, supportedLayer.layerName) == 0)
			{
				found = true;
				break;
			}
		}

		if (found == false)
		{
			LOG_WARNING("Layer %s is not supported", layer);
			return false;
		}
	}

	return true;
}
