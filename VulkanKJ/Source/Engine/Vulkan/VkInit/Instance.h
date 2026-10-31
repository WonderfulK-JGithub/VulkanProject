#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

#define USE_VALIDATION_LAYER

namespace VkInit
{
	vk::Instance MakeInstance(const char* aApplicationName);

	uint32_t GetInstanceVersion();

	bool IsExtensionsSupported(const std::vector<const char*>& aExtensions, const std::vector<const char*>& aLayers);
}