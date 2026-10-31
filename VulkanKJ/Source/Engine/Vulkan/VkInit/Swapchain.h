#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <optional>
#include "../VkMemory.h"
#include <glm/mat4x4.hpp>
#include "Pipeline.h"
#include <unordered_map>
#include "RenderPass.h"

namespace VkInit
{
	struct SwapchainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct CameraVectors
	{
		glm::vec4 right;
		glm::vec4 up;
		glm::vec4 forward;
	};

	struct CameraMatrices
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	struct ModelInstance
	{
		glm::mat4 transform;
	};

	struct SwapchainFrame
	{
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		
		int width;
		int height;

		vk::CommandBuffer commandBuffer;

		vk::Fence inFlightFence;
		vk::Semaphore imageAvailable;
		vk::Semaphore renderFinnished;

		void Destroy();
	};

	struct SwapchainInfo
	{
		vk::SwapchainKHR swapchain;
		std::vector<SwapchainFrame> frames;
		std::vector<vk::Image> images;
		vk::Format format;
		vk::Format depthFormat;
		vk::Extent2D extent;
		uint32_t minImages;
	};

	

	vk::SurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& aFormats);
	vk::PresentModeKHR ChooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& aPresentModes);
	vk::Extent2D ChooseSwapchainExtent(uint32_t aWidth, uint32_t aHeight, vk::SurfaceCapabilitiesKHR aCapabilities);

	SwapchainSupportDetails GetSwapchainSupport(const vk::PhysicalDevice& aDevice, const vk::SurfaceKHR& aSurface);

	SwapchainInfo CreateSwapchain(vk::Device aLogicalDevice, vk::PhysicalDevice aPhysicalDevice, const vk::SurfaceKHR& aSurface, int aWidth, int aHeight);
}