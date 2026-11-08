#include "SceneRenderer.h"

#include "Engine.h"
#include "Vulkan/Buffer/VulkanBuffer.h"
#include "Vulkan/VkInit/Descriptors.h"
#include "AssetManagement/AssetRegistry.h"
#include "AssetManagement/ModelAsset.h"
#include "Vulkan/VkInit/PipelineLayout.h"

namespace SceneRenderer
{
    static vk::DescriptorPool localGlobalDescriptorPool;
	static vk::DescriptorPool localModelInstanceDescriptorPool;
}

void SceneRenderer::CreateSceneImageInfo(SceneImageInfo& aInfoOut,RenderImage& aFinalRenderImage, const vk::Extent2D aSize)
{
	aInfoOut.finalRenderImage = &aFinalRenderImage;
	aInfoOut.depthImage.Init(aSize.width,aSize.height,vk::Format::eD32Sfloat,true);
	aInfoOut.intermediateImage.Init(aSize.width,aSize.height,vk::Format::eB8G8R8A8Unorm,false);
	
	aInfoOut.myCubemapStage.AddRenderImage(aInfoOut.intermediateImage);
	aInfoOut.myCubemapStage.CreateFrameBuffers(Vulkan::RenderStageType::Cubemap,aSize);
	
	aInfoOut.myForwardStage.AddRenderImage(aInfoOut.intermediateImage);
	aInfoOut.myForwardStage.AddRenderImage(aInfoOut.depthImage);
	aInfoOut.myForwardStage.CreateFrameBuffers(Vulkan::RenderStageType::Forward,aSize);
	
	aInfoOut.myCopyStage.AddRenderImage(aFinalRenderImage);
	aInfoOut.myCopyStage.CreateFrameBuffers(aFinalRenderImage.IsSwapchainImage() ? Vulkan::RenderStageType::FullscreenSwapchain :
		Vulkan::RenderStageType::FullscreenOffscreen,aSize);
}

void SceneRenderer::CreateSceneObjectInfo(SceneObjectInfo& aInfoOut)
{
	aInfoOut.cameraDataBuffer.Init(sizeof(VkInit::CameraData),vk::BufferUsageFlagBits::eUniformBuffer);
	aInfoOut.instanceBuffer.Init(sizeof(VkInit::ModelInstance) * 1024,vk::BufferUsageFlagBits::eStorageBuffer);
	aInfoOut.lineVertexBuffer.Init(sizeof(Vulkan::LineVertex) * MAX_LINE_COUNT * 2,vk::BufferUsageFlagBits::eVertexBuffer);
	
	aInfoOut.descriptorSetCollection.globalDescriptorSet.Init(localGlobalDescriptorPool,
		VkInit::GetDescriptorSetLayout(VkInit::DescriptorLayoutType::Camera));
	aInfoOut.descriptorSetCollection.modelInstanceDescriptorSet.Init(localModelInstanceDescriptorPool,
		VkInit::GetDescriptorSetLayout(VkInit::DescriptorLayoutType::ModelInstance));
	
	aInfoOut.descriptorWriter.AddDescriptorInfo(aInfoOut.descriptorSetCollection.globalDescriptorSet,aInfoOut.cameraDataBuffer,
		vk::DescriptorType::eUniformBuffer,0);
	aInfoOut.descriptorWriter.AddDescriptorInfo(aInfoOut.descriptorSetCollection.modelInstanceDescriptorSet,aInfoOut.instanceBuffer,
		vk::DescriptorType::eStorageBuffer,0);
}

void SceneRenderer::DestroySceneImageInfo(SceneImageInfo& aInfoOut)
{
	aInfoOut.myCubemapStage.Destroy();
	aInfoOut.myForwardStage.Destroy();
	aInfoOut.myCopyStage.Destroy();
	
	aInfoOut.intermediateImage.Destroy();
	aInfoOut.depthImage.Destroy();
}

void SceneRenderer::DestroySceneObjectInfo(SceneObjectInfo& aInfoOut)
{
	aInfoOut.descriptorWriter.Reset();
	
	aInfoOut.cameraDataBuffer.Destroy();
	aInfoOut.instanceBuffer.Destroy();
	aInfoOut.lineVertexBuffer.Destroy();
}

void SceneRenderer::Init()
{
	Engine& engine = Engine::GetInstance();
	
	{
		VkInit::DescriptorSetLayoutData input;
		input.count = 1;
		input.types.push_back(vk::DescriptorType::eUniformBuffer);
		localGlobalDescriptorPool = VkInit::MakeDescriptorPool(engine.GetLogicalDevice(), 100, input);
	}
	
	{
		VkInit::DescriptorSetLayoutData input;
		input.count = 1;
		input.types.push_back(vk::DescriptorType::eStorageBuffer);
		localModelInstanceDescriptorPool = VkInit::MakeDescriptorPool(engine.GetLogicalDevice(), 100, input);
	}
}

void SceneRenderer::Destroy()
{
	Engine::GetInstance().GetLogicalDevice().destroyDescriptorPool(localGlobalDescriptorPool);
	Engine::GetInstance().GetLogicalDevice().destroyDescriptorPool(localModelInstanceDescriptorPool);
}

void SceneRenderer::Render(vk::CommandBuffer aCommandBuffer,SceneImageInfo& aImageInfo, const SceneObjectInfo& aObjectInfo)
{
	Internal::Prepare(aCommandBuffer,aObjectInfo);
	Internal::RenderCubemap(aCommandBuffer,aImageInfo,aObjectInfo);
	Internal::RenderForward(aCommandBuffer,aImageInfo,aObjectInfo);
	Internal::CopyToFinalImage(aCommandBuffer,aImageInfo,aObjectInfo);
}

void SceneRenderer::Internal::Prepare(vk::CommandBuffer aCommandBuffer,const SceneObjectInfo& aObjectInfo)
{
	{
		memcpy(aObjectInfo.cameraDataBuffer.GetWriteLocation(), &aObjectInfo.cameraData, sizeof(aObjectInfo.cameraData));
	}
	
	memcpy(aObjectInfo.lineVertexBuffer.GetWriteLocation(), aObjectInfo.linePoints.data(), sizeof(Vulkan::LineVertex) * aObjectInfo.linePoints.size());
	
	const vk::Buffer vertexBuffer[]{ aObjectInfo.lineVertexBuffer.GetBuffer().buffer };
	vk::DeviceSize offsets[]{ 0 };
	aCommandBuffer.bindVertexBuffers(1, 1, vertexBuffer, offsets);
	
	auto& instanceMap = aObjectInfo.instances.GetInstances();
	
	size_t startInstance = 0;
	for (uint32_t i = 0; i < instanceMap.size(); ++i)
	{
		const size_t batchSize = instanceMap.data()[i].size();
		
		if (batchSize > 0)
		{
			void* dst = static_cast<char*>(aObjectInfo.instanceBuffer.GetWriteLocation()) + startInstance * sizeof(VkInit::ModelInstance);
			memcpy(dst, instanceMap.data()[i].data(), 
				batchSize * sizeof(VkInit::ModelInstance));
			startInstance += batchSize;
		}
		
	}
	
	aObjectInfo.descriptorWriter.UpdateDescriptorSets();
}

void SceneRenderer::Internal::RenderCubemap(const vk::CommandBuffer aCommandBuffer,SceneImageInfo& aImageInfo,
	const SceneObjectInfo& aObjectInfo)
{
	aImageInfo.intermediateImage.TransitionToTarget(aCommandBuffer);
	aImageInfo.myCubemapStage.StartRenderPass(aCommandBuffer,VkInit::GetRenderPass(Vulkan::RenderStageType::Cubemap),
		Vulkan::GetAllResolutions().at(aImageInfo.resolutionId));
	
	Wonder::MaterialAsset& cubemapMaterial = Wonder::GetMaterialRegistry().AccessAssetFromID(aObjectInfo.cubemapMaterial);
	
	Vulkan::BindPipelineLayout(aCommandBuffer,aObjectInfo.descriptorSetCollection,Vulkan::PipelineLayoutType::DefaultPipeline);

	cubemapMaterial.Bind(aCommandBuffer,aImageInfo.resolutionId);
	aCommandBuffer.draw(6, 1, 0, 0);

	aCommandBuffer.endRenderPass();
}

void SceneRenderer::Internal::RenderForward(vk::CommandBuffer aCommandBuffer, SceneImageInfo& aImageInfo,
	const SceneObjectInfo& aObjectInfo)
{
	aImageInfo.depthImage.TransitionToTarget(aCommandBuffer);
	aImageInfo.myForwardStage.StartRenderPass(aCommandBuffer,VkInit::GetRenderPass(Vulkan::RenderStageType::Forward),
		Vulkan::GetAllResolutions().at(aImageInfo.resolutionId));

	auto& instanceMap = aObjectInfo.instances.GetInstances();
	
	Vulkan::BindPipelineLayout(aCommandBuffer,aObjectInfo.descriptorSetCollection,Vulkan::PipelineLayoutType::DefaultPipeline);

	uint32_t startInstance = 0;
	
	auto& materials = Wonder::GetMaterialRegistry().AccessAssetMap();
	for (uint32_t i = 0; i < instanceMap.size(); ++i)
	{
		const uint32_t instanceCount = static_cast<uint32_t>(instanceMap.data()[i].size());
		
		if (instanceCount == 0)
		{
			continue;
		}

		Vulkan::MeshId meshId = instanceMap.dataKeys()[i].meshId;

		auto drawInfo = Vulkan::MeshPool::GetInstance().GetVertexDrawInfo(meshId);

		Wonder::MaterialAsset& material = materials.data()[instanceMap.dataKeys()[i].materialIndex];
		material.Bind(aCommandBuffer,aImageInfo.resolutionId);

		aCommandBuffer.drawIndexed(drawInfo.indexCount, instanceCount, drawInfo.firstIndex, drawInfo.vertexOffset, startInstance);
	
		startInstance += instanceCount;
	}
	
	if (aObjectInfo.linePoints.empty() == false)
	{
		Wonder::GetMaterialRegistry().AccessAssetFromID(Wonder::LINE_MATERIAL).Bind(aCommandBuffer,aImageInfo.resolutionId);
		aCommandBuffer.draw(static_cast<uint32_t>(aObjectInfo.linePoints.size()),1,0,0);
	}
	

	aCommandBuffer.endRenderPass();
}

void SceneRenderer::Internal::CopyToFinalImage(vk::CommandBuffer aCommandBuffer,SceneImageInfo& aImageInfo,const SceneObjectInfo& aObjectInfo)
{
	aImageInfo.intermediateImage.TransitionToResource(aCommandBuffer);
	
	auto& engine = Engine::GetInstance();
	if (aImageInfo.finalRenderImage->IsSwapchainImage())
	{
		aImageInfo.myCopyStage.StartRenderPass(aCommandBuffer,VkInit::GetRenderPass(Vulkan::RenderStageType::FullscreenSwapchain),
			Vulkan::GetAllResolutions().at(aImageInfo.resolutionId));
		
		auto& material = Wonder::GetMaterialRegistry().AccessAssetFromID(Wonder::FULLSCREEN_COPY_SWAPCHAIN);
		
		Vulkan::BindPipelineLayout(aCommandBuffer,aObjectInfo.descriptorSetCollection,Vulkan::PipelineLayoutType::DefaultPipeline);
		
		material.RemoveRenderImages();
		material.AddRenderImage(aImageInfo.intermediateImage.GetRenderImageId(engine.GetCurrentSwapchainIndex()));
		material.Bind(aCommandBuffer,aImageInfo.resolutionId);
	}
	else
	{
		aImageInfo.myCopyStage.StartRenderPass(aCommandBuffer,VkInit::GetRenderPass(Vulkan::RenderStageType::FullscreenOffscreen),
			Vulkan::GetAllResolutions().at(aImageInfo.resolutionId));
		
		auto& material = Wonder::GetMaterialRegistry().AccessAssetFromID(Wonder::FULLSCREEN_COPY_OFFSCREEN);
		
		Vulkan::BindPipelineLayout(aCommandBuffer,aObjectInfo.descriptorSetCollection,Vulkan::PipelineLayoutType::DefaultPipeline);
		
		material.RemoveRenderImages();
		material.AddRenderImage(aImageInfo.intermediateImage.GetRenderImageId(engine.GetCurrentSwapchainIndex()));
		material.Bind(aCommandBuffer,aImageInfo.resolutionId);
	}
	
	aCommandBuffer.draw(6, 1, 0, 0);
	aCommandBuffer.endRenderPass();
}

