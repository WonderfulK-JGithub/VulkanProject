#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "Vulkan/VkInit/Swapchain.h"
#include "Vulkan/VkScene.h"
#include "Vulkan/VkMesh.h"
#include "Vulkan/VkInit/Device.h"
#include "Vulkan/VulkanImGui.h"
#include "Vulkan/Image/TextureDescriptor.h"
#include "Rendering/RenderImage.h"
#include "Rendering/RenderStage.h"
#include "Rendering/SceneRenderer.h"

class Engine {

public:

	Engine(int aWidth, int aHeight, GLFWwindow* aWindow);

	~Engine();

	void BeginImgui();
	void RenderImGui();
	
	void BeginRender();
	void EndRender();
	
	void RenderScene(VkScene* aScene);

	static Engine& GetInstance();
	
	float GetDeltaTime() const;

	vk::Instance GetVulkanInstance() const;
	vk::PhysicalDevice GetPhysicalDevice() const;
	vk::Device GetLogicalDevice() const;
	vk::Queue GetGraphicsQueue() const;
	vk::SurfaceKHR GetSurface() const;
	const std::vector<VkInit::SwapchainFrame>& GetSwapchainFrames();
	uint32_t GetSwapchainMinImages() const;
	vk::CommandPool GetCommandPool() const;
	vk::CommandBuffer GetMainCommandBuffer() const;
	vk::CommandBuffer GetCurrentCommandBuffer() const;
	vk::DescriptorPool GetTextureDescriptorPool() const;
	

	vk::Format GetSwapchainColorFormat() const;
	vk::Format GetSwapchainDepthFormat() const;
	vk::Extent2D GetSwapchainExtent() const;
	
	uint32_t GetCurrentSwapchainIndex() const;

	VkInit::QueueFamilyIndices GetQueueFamilyIndices() const;
	
	void WaitForIdle();
private:
	ImGUIRenderer myImguiRenderer;

	//glfw window parameters
	int width{ 640 };
	int height{ 480 };
	GLFWwindow* myWindow{ nullptr };

	vk::Instance myInstance{ nullptr };

	vk::DebugUtilsMessengerEXT myDebugMessenger{ nullptr };

	vk::detail::DispatchLoaderDynamic myDLD;
	vk::SurfaceKHR mySurface;

	vk::PhysicalDevice myPhysicalDevice{ nullptr };
	vk::Device myLogicalDevice{ nullptr };
	vk::Queue myGraphicsQueue{ nullptr };
	vk::Queue myPresentQueue{ nullptr };
	VkInit::QueueFamilyIndices myQueFamilyIndices;

	VkInit::SwapchainInfo mySwapchainInfo;
	
	RenderImage mySwapchainRenderImage;
	// RenderImage myDepthRenderImage;
	// RenderStage myCubemapStage;
	RenderStage myImGuiStage;

	SceneRenderer::SceneObjectInfo mySceneObjectInfo;
	SceneRenderer::SceneImageInfo mySceneImageInfo;
	
	std::vector<VkInit::ModelInstance> myModelInstances;
	
	vk::CommandPool myCommandPool;
	vk::CommandBuffer myMainCommandBuffer;

	int myMaxFramesInFlight{};
	int myFrameNumber{};
	uint32_t myImageIndex{};
	
	vk::DescriptorPool myTextureDescriptorPool;

	Vulkan::MeshPool* myMeshPool;

	Vulkan::TextureDescriptor myTextureDescriptor;
	
	double myLastTime;
	double myCurrentTime;
	uint32_t myFrameCount;
	float myDeltaTime;
	
	glm::vec3 myCameraRotation;
	
	void CalculateFrameRate();

	void MakeDevice();
	void MakeSwapchain();
	void RecreateSwapchain();
	
	void MakeInstance();

	void FinalizeSetup();
	void MakeFrameResources();
	void MakeRenderTargets();

	void MakeAssets();
	void PrepareScene(vk::CommandBuffer aCommandBuffer) const;

	void DestroySwapchain();
	void DestroyRenderTargets();
};
