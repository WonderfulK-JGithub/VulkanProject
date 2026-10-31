#include "Engine.h"

#include <glm/gtx/euler_angles.hpp>

#include "Vulkan/VkInit/Instance.h"
#include "Vulkan/DeviceLogging.h"
#include "Vulkan/VkInit/Device.h"
#include <vulkan/vulkan_core.h>
#include "Vulkan/VkInit/Swapchain.h"
#include "Vulkan/VkInit/VkCommands.h"
#include "Vulkan/VkInit/VkSync.h"
#include "Vulkan/RenderStructs.h"
#include "AssetManagement/AssetRegistry.h"
#include "AssetManagement/TextureAsset.h"
#include "AssetManagement/Loaders/MaterialBuildQueue.h"
#include "AssetManagement/Loaders/TextureLoader.h"
#include "ECS/Reflection/ComponentType.h"
#include "ID/IdFunctions.h"
#include "Input/KeyboardInput.h"
#include "Vulkan/Image/Sampler.h"
#include "Vulkan/VkInit/Descriptors.h"
#include "Vulkan/VkInit/RenderPass.h"
#include "Shaders/Shaders.h"
#include "Util/FileManagement/FileWatcher.h"
#include "Util/FileManagement/Settings.h"
#include "Util/Logging/Logging.h"
#include "Vulkan/VkInit/PipelineLayout.h"

static Engine* localCurrentEngine;

namespace Wonder
{
	void EnsureComponentTypesAreLoaded();
}


Engine::Engine(int aWidth, int aHeight, GLFWwindow* aWindow) {
	width = aWidth;
	height = aHeight;
	myWindow = aWindow;

	localCurrentEngine = this;

	Wonder::LoadSettings();
	
	LOG_INFO("%s","Making a graphics engine");

	MakeInstance();

	MakeDevice();

	FinalizeSetup();

	SceneRenderer::Init();
	SceneRenderer::CreateSceneObjectInfo(mySceneObjectInfo);
	
	MakeAssets();
	
	Wonder::LoadIdCounters();

	Wonder::EnsureComponentTypesAreLoaded();
	Wonder::ComponentTypeBase::FinalizeTypeRegistry();
	
	Wonder::FileWatcher::Init();
}

void Engine::CalculateFrameRate()
{
	myCurrentTime = glfwGetTime();
	double delta = myCurrentTime - myLastTime;

	myDeltaTime = static_cast<float>(delta);
	myLastTime = myCurrentTime;
	++myFrameCount;
}

void Engine::MakeDevice()
{
	myPhysicalDevice = VkInit::ChoosePhysicalDevice(myInstance);
	myQueFamilyIndices = VkInit::FindQueueFamilies(myPhysicalDevice, mySurface);
	
	myLogicalDevice = VkInit::CreateLogicalDevice(myPhysicalDevice,mySurface);
	myGraphicsQueue = VkInit::GetGraphicsQueue(myPhysicalDevice,myLogicalDevice,mySurface);
	myPresentQueue = VkInit::GetPresentQueue(myPhysicalDevice, myLogicalDevice, mySurface);

	MakeSwapchain();
}

void Engine::MakeSwapchain()
{
	width = 0;
	height = 0;

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(myWindow, &width, &height);
		glfwWaitEvents();
	}
	
	mySwapchainInfo = VkInit::CreateSwapchain(myLogicalDevice, myPhysicalDevice, mySurface, width, height);
	
	myMaxFramesInFlight = static_cast<int>(mySwapchainInfo.frames.size());

	for (VkInit::SwapchainFrame& frame : mySwapchainInfo.frames)
	{
		frame.logicalDevice = myLogicalDevice;
		frame.physicalDevice = myPhysicalDevice;
		frame.width = width;
		frame.height = height;
	}
	
	Vulkan::SetResolution(Vulkan::SWAPCHAIN_RESOLUTION_ID,mySwapchainInfo.extent);
	
	myFrameNumber = 0;
}

void Engine::RecreateSwapchain()
{
	DestroySwapchain();
	DestroyRenderTargets();
	MakeSwapchain();
	MakeRenderTargets();
	MakeFrameResources();

	VkInit::CommandBufferInput commandInput{ myLogicalDevice,myCommandPool,mySwapchainInfo.frames };
	VkInit::MakeFrameCommandBuffers(commandInput);
}


void Engine::MakeInstance()
{
	myInstance = VkInit::MakeInstance("application name");

	myDLD = vk::detail::DispatchLoaderDynamic(myInstance, vkGetInstanceProcAddr);

#ifdef USE_VALIDATION_LAYER
	myDebugMessenger = VkInit::MakeDebugMessenger(myInstance, myDLD);
#endif
	
	VkSurfaceKHR cStyleSurface;
	if (glfwCreateWindowSurface(myInstance, myWindow, nullptr, &cStyleSurface) != VK_SUCCESS)
	{
		LOG_ERROR("%s","Failed to create window surface!");
	}
	else
	{
		LOG_INFO("%s","Successfully created window surface!");
	}
	mySurface = cStyleSurface;
}

void Engine::FinalizeSetup()
{
	ShaderReflection::InitializeDefaultValues();
	
	VkInit::CreateStageDescriptorSetLayouts();
	Vulkan::CreateVertexInputInfos();
	VkInit::CreateRenderPasses(myLogicalDevice, mySwapchainInfo.format, mySwapchainInfo.depthFormat);
	Vulkan::CreateSamplers();
	
	VkInit::DescriptorSetLayoutData descriptorSetInput;
	descriptorSetInput.count = 1;
	descriptorSetInput.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	myTextureDescriptorPool = VkInit::MakeDescriptorPool(myLogicalDevice, 128, descriptorSetInput);
	myTextureDescriptor.Init();
	
	Vulkan::CreatePipelineLayouts();
	
	myImguiRenderer.Init(myWindow, *this);
	
	
	
	MakeRenderTargets();

	myCommandPool = VkInit::MakeCommandPool(myLogicalDevice, myPhysicalDevice, mySurface);

	VkInit::CommandBufferInput commandInput{ myLogicalDevice,myCommandPool,mySwapchainInfo.frames };
	myMainCommandBuffer = VkInit::MakeCommandBuffer(commandInput.device, commandInput.commandPool);

	VkInit::MakeFrameCommandBuffers(commandInput);

	MakeFrameResources();
}

void Engine::MakeFrameResources()
{
	for (uint32_t i = 0; i < mySwapchainInfo.frames.size(); i++)
	{
		auto& frame = mySwapchainInfo.frames[i];

		frame.inFlightFence = VkInit::CreateFence(myLogicalDevice);
		frame.imageAvailable = VkInit::CreateSemaphore(myLogicalDevice);
		frame.renderFinnished = VkInit::CreateSemaphore(myLogicalDevice);
	}
}

void Engine::MakeRenderTargets()
{
	mySwapchainRenderImage.InitFromSwapchainImages(mySwapchainInfo.images);
	
	myImGuiStage.AddRenderImage(mySwapchainRenderImage);
	myImGuiStage.CreateFrameBuffers(Vulkan::RenderStageType::ImGui,mySwapchainInfo.extent);
	
	SceneRenderer::CreateSceneImageInfo(mySceneImageInfo,mySwapchainRenderImage,mySwapchainInfo.extent);
	mySceneImageInfo.resolutionId = Vulkan::SWAPCHAIN_RESOLUTION_ID;
}

void Engine::MakeAssets()
{
	myMeshPool = new Vulkan::MeshPool();
	
	Wonder::RegisterDefaultTextures();
	Wonder::RegisterDefaultModels();
	Wonder::RegisterDefaultMaterials();
	Wonder::SearchAndRegisterAllAssets();
}

void Engine::PrepareScene(vk::CommandBuffer aCommandBuffer) const
{
	vk::Buffer vertexBuffer[]{ myMeshPool->myVertexBuffer.buffer };
	vk::DeviceSize offsets[]{ 0 };
	aCommandBuffer.bindVertexBuffers(0, 1, vertexBuffer, offsets);
	aCommandBuffer.bindIndexBuffer(myMeshPool->myIndexBuffer.buffer, 0, vk::IndexType::eUint32);
}

void Engine::DestroySwapchain()
{
	myLogicalDevice.waitIdle();

	for (auto& frame : mySwapchainInfo.frames)
	{
		frame.Destroy();
	}

	myLogicalDevice.destroySwapchainKHR(mySwapchainInfo.swapchain);
}

void Engine::DestroyRenderTargets()
{
	myImGuiStage.Destroy();
	
	mySwapchainRenderImage.Destroy();
	
	SceneRenderer::DestroySceneImageInfo(mySceneImageInfo);
}

Engine::~Engine() {

	Wonder::FileWatcher::Destroy();
	
	myLogicalDevice.waitIdle();

	Vulkan::DestroyAllModules(myLogicalDevice);

	myLogicalDevice.destroyCommandPool(myCommandPool);

	VkInit::DestroyRenderPasses(myLogicalDevice);

	DestroySwapchain();
	DestroyRenderTargets();
	
	SceneRenderer::DestroySceneObjectInfo(mySceneObjectInfo);
	SceneRenderer::Destroy();

	VkInit::DestroyStageDescriptorSetLayouts();

	delete myMeshPool;

	// for (auto& [key, texture] : myMeshTextures)
	// {
	// 	texture.Destroy();
	// }
	
	myTextureDescriptor.Destroy();
	
	auto& materials = Wonder::GetMaterialRegistry().AccessAssetMap();
	for (uint32_t i = 0; i < materials.size(); ++i)
	{
		materials.data()[i].Destroy();
	}
	
	auto& textureAssets = Wonder::GetTextureRegistry().AccessAssetMap();
	for (uint32_t i = 0; i < textureAssets.size(); ++i)
	{
		textureAssets.data()[i].Unload();
	}
	

	Vulkan::DestroySamplers();
	Vulkan::DestroyPipelineLayouts();
	
	myImguiRenderer.Destroy();

	
	
	myLogicalDevice.destroyDescriptorPool(myTextureDescriptorPool);

	myLogicalDevice.destroy();

	myInstance.destroySurfaceKHR(mySurface);
	
#ifdef USE_VALIDATION_LAYER
	myInstance.destroyDebugUtilsMessengerEXT(myDebugMessenger, nullptr, myDLD);
#endif
	
	LOG_INFO("Goodbye see you!");
	
	//terminate glfw
	glfwTerminate();

	
	myInstance.destroy();
}

void Engine::BeginImgui()
{
	myImguiRenderer.BeginFrame();
}

void Engine::RenderImGui()
{
	Goose::Logging::Internal::DisplayConsole();
	Goose::Logging::Internal::DisplayStats();
	
	const vk::CommandBuffer commandBuffer = mySwapchainInfo.frames[myImageIndex].commandBuffer;
	myImGuiStage.StartRenderPass(commandBuffer,VkInit::GetRenderPass(Vulkan::RenderStageType::ImGui),mySwapchainInfo.extent);
	
	myImguiRenderer.EndFrame(commandBuffer);
	
	commandBuffer.endRenderPass();
}

void Engine::BeginRender()
{
	CalculateFrameRate();
	Wonder::FileWatcher::CheckForUpdates();
	
	if (Wonder::TextureLoader::TexturesNeedsUpdate() || myTextureDescriptor.NeedsDescriptorUpdate()
		|| myMeshPool->NeedsUpdate() || MaterialBuildQueue::NeedsRebuild())
	{
		WaitForIdle();
	}
	
	if (Wonder::TextureLoader::TexturesNeedsUpdate())
	{
		Wonder::TextureLoader::UpdateTextures();
	}
	
	if (myTextureDescriptor.NeedsDescriptorUpdate())
	{
		myTextureDescriptor.UpdateDescriptorSet();
	}
	
	if (myMeshPool->NeedsUpdate())
	{
		Vulkan::FinalizeInput input;
		input.logicalDevice = myLogicalDevice;
		input.physicalDevice = myPhysicalDevice;
		input.queue = myGraphicsQueue;
		input.commandBuffer = myMainCommandBuffer;

		myMeshPool->Finalize(input);
	}
	
	if (MaterialBuildQueue::NeedsRebuild())
	{
		MaterialBuildQueue::RebuildQueuedMaterials();
	}
	
	[[maybe_unused]] vk::Result fenceResult = myLogicalDevice.waitForFences(1, &mySwapchainInfo.frames[myFrameNumber].inFlightFence, VK_TRUE, UINT64_MAX);
	
	
	try
	{
		myImageIndex = myLogicalDevice.acquireNextImageKHR(mySwapchainInfo.swapchain, UINT64_MAX, mySwapchainInfo.frames[myFrameNumber].imageAvailable, {}).value;
	}
	catch (vk::OutOfDateKHRError)
	{
		LOG_INFO("%s","Recreating swapchain");
		RecreateSwapchain();
		return;
	}

	vk::CommandBuffer commandBuffer = mySwapchainInfo.frames[myImageIndex].commandBuffer;

	commandBuffer.reset();

	

	vk::CommandBufferBeginInfo commandBeginInfo{};

	try
	{
		commandBuffer.begin(commandBeginInfo);
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to begin commandBuffer, %s",err.what());
	}
	
	PrepareScene(commandBuffer);
}

void Engine::EndRender()
{
	vk::CommandBuffer commandBuffer = mySwapchainInfo.frames[myImageIndex].commandBuffer;
	
	try
	{
		commandBuffer.end();
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to end commandBuffer, %s",err.what());
	}


	vk::PipelineStageFlags waitStages[]{ vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &mySwapchainInfo.frames[myFrameNumber].imageAvailable;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &mySwapchainInfo.frames[myFrameNumber].renderFinnished;

	[[maybe_unused]] vk::Result fenceResult = myLogicalDevice.resetFences(1, &mySwapchainInfo.frames[myFrameNumber].inFlightFence);
	try
	{
		myGraphicsQueue.submit(submitInfo, mySwapchainInfo.frames[myFrameNumber].inFlightFence);
	}
	catch (vk::SystemError err)
	{
		LOG_ERROR("Failed to submit, %s",err.what());
	}

	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 01;
	presentInfo.pWaitSemaphores = &mySwapchainInfo.frames[myFrameNumber].renderFinnished;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mySwapchainInfo.swapchain;
	presentInfo.pImageIndices = &myImageIndex;

	vk::Result presentResult;

	myImguiRenderer.Present();
	
	try
	{
		presentResult = myPresentQueue.presentKHR(presentInfo);
	}
	catch(vk::OutOfDateKHRError err)
	{
		presentResult = vk::Result::eErrorOutOfDateKHR;
	}

	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
	{
		LOG_INFO("%s","Recreating swapchain");
		RecreateSwapchain();
		return;
	}

	myFrameNumber = (myFrameNumber + 1) % myMaxFramesInFlight;
}

void Engine::RenderScene(VkScene* aScene)
{
	const glm::mat4 view = glm::yawPitchRoll(myCameraRotation.x,myCameraRotation.y,myCameraRotation.z);
	{
		if (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::Space))
		{
			myCameraRotation.y += myDeltaTime * 0.1f;
		}
		
		glm::mat4 projection = glm::perspective(
			glm::radians(45.f),
			static_cast<float>(mySwapchainInfo.extent.width) / static_cast<float>(mySwapchainInfo.extent.height),
			0.1f,
			10.f);
		projection[1][1] *= -1.f;
		
		mySceneObjectInfo.cameraMatrixData.view = view;
		mySceneObjectInfo.cameraMatrixData.projection = projection;
		mySceneObjectInfo.cameraMatrixData.viewProjection = projection * view;
	}

	{
		
		mySceneObjectInfo.cameraVectorData.right = view[0];
		mySceneObjectInfo.cameraVectorData.up = view[1];
		mySceneObjectInfo.cameraVectorData.forward = view[2];
	}
	
	mySceneObjectInfo.cubemapMaterial = Wonder::TEST_CUBEMAP_MATERIAL;
	
	mySceneObjectInfo.instances.ClearInstances();
	
	for (auto& [type, value] : aScene->myMeshPositions)
	{
		for (const glm::vec3& position : value)
		{
			switch (type)
			{
				case VkScene::MeshType::Triangle:
					mySceneObjectInfo.instances.AddInstance(Wonder::TEST_MATERIAL_1, 
					Wonder::GetModelRegistry().AccessAssetFromID(Wonder::TRIANGLE_MODEL).GetMeshes().front().id,
					{glm::translate(glm::mat4(1.f), position)});
					break;
				case VkScene::MeshType::Square:
					mySceneObjectInfo.instances.AddInstance(Wonder::TEST_MATERIAL_2, 
					Wonder::GetModelRegistry().AccessAssetFromID(Wonder::SQUARE_MODEL).GetMeshes().front().id,
					{glm::translate(glm::mat4(1.f), position)});
					break;
				case VkScene::MeshType::Obj:
					mySceneObjectInfo.instances.AddInstance(Wonder::TEST_MATERIAL_3, 
					Wonder::GetModelRegistry().AccessAssetFromID(Wonder::TEST_OBJ_MODEL).GetMeshes().front().id,
					{glm::translate(glm::mat4(1.f), position)});
					break;
			}
		}
	}
	
	vk::CommandBuffer commandBuffer = mySwapchainInfo.frames[myImageIndex].commandBuffer;
	
	SceneRenderer::Render(commandBuffer,mySceneImageInfo,mySceneObjectInfo);
}

Engine& Engine::GetInstance()
{
	return *localCurrentEngine;
}

float Engine::GetDeltaTime() const
{
	return myDeltaTime;
}

vk::Instance Engine::GetVulkanInstance() const
{
	return myInstance;
}

vk::PhysicalDevice Engine::GetPhysicalDevice() const
{
	return myPhysicalDevice;
}

vk::Device Engine::GetLogicalDevice() const
{
	return myLogicalDevice;
}

vk::Queue Engine::GetGraphicsQueue() const
{
	return myGraphicsQueue;
}

vk::SurfaceKHR Engine::GetSurface() const
{
	return mySurface;
}

const std::vector<VkInit::SwapchainFrame>& Engine::GetSwapchainFrames()
{
	return mySwapchainInfo.frames;
}

uint32_t Engine::GetSwapchainMinImages() const
{
	return mySwapchainInfo.minImages;
}

vk::CommandPool Engine::GetCommandPool() const
{
	return myCommandPool;
}

vk::CommandBuffer Engine::GetMainCommandBuffer() const
{
	return myMainCommandBuffer;
}

vk::CommandBuffer Engine::GetCurrentCommandBuffer() const
{
	return mySwapchainInfo.frames[myImageIndex].commandBuffer;
}

vk::DescriptorPool Engine::GetTextureDescriptorPool() const
{
	return myTextureDescriptorPool;
}

vk::Format Engine::GetSwapchainColorFormat() const
{
	return mySwapchainInfo.format;
}

vk::Format Engine::GetSwapchainDepthFormat() const
{
	return mySwapchainInfo.depthFormat;
}

vk::Extent2D Engine::GetSwapchainExtent() const
{
	return mySwapchainInfo.extent;
}

uint32_t Engine::GetCurrentSwapchainIndex() const
{
	return myFrameNumber;
}

VkInit::QueueFamilyIndices Engine::GetQueueFamilyIndices() const
{
	return myQueFamilyIndices;
}

void Engine::WaitForIdle()
{
	myGraphicsQueue.waitIdle();
	myLogicalDevice.waitIdle();
}
