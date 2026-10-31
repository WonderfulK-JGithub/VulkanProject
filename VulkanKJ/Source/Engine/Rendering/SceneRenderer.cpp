#include "SceneRenderer.h"

#include "Engine.h"
#include "Vulkan/Buffer/VulkanBuffer.h"
#include "Vulkan/VkInit/Descriptors.h"
#include "AssetManagement/AssetRegistry.h"
#include "AssetManagement/ModelAsset.h"
#include "Vulkan/VkInit/PipelineLayout.h"

namespace SceneRenderer
{
    static vk::DescriptorPool localBufferDescriptorPool;
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
	aInfoOut.cameraMatrixBuffer.Init(sizeof(VkInit::CameraMatrices),vk::BufferUsageFlagBits::eUniformBuffer);
	aInfoOut.cameraVectorBuffer.Init(sizeof(VkInit::CameraVectors),vk::BufferUsageFlagBits::eUniformBuffer);
	aInfoOut.instanceBuffer.Init(sizeof(VkInit::ModelInstance) * 1024,vk::BufferUsageFlagBits::eStorageBuffer);
	aInfoOut.lineVertexBuffer.Init(sizeof(Vulkan::LineVertex) * MAX_LINE_COUNT * 2,vk::BufferUsageFlagBits::eVertexBuffer);
	
	aInfoOut.cubemapDescriptorSet.Init(localBufferDescriptorPool,VkInit::GetStageDescriptorSetLayout(Vulkan::RenderStageType::Cubemap));
	aInfoOut.forwardDescriptorSet.Init(localBufferDescriptorPool,VkInit::GetStageDescriptorSetLayout(Vulkan::RenderStageType::Forward));
	
	aInfoOut.descriptorWriter.AddDescriptorInfo(aInfoOut.cubemapDescriptorSet,aInfoOut.cameraVectorBuffer,vk::DescriptorType::eUniformBuffer,0);
	
	aInfoOut.descriptorWriter.AddDescriptorInfo(aInfoOut.forwardDescriptorSet,aInfoOut.cameraMatrixBuffer,vk::DescriptorType::eUniformBuffer,0);
	aInfoOut.descriptorWriter.AddDescriptorInfo(aInfoOut.forwardDescriptorSet,aInfoOut.instanceBuffer,vk::DescriptorType::eStorageBuffer,1);
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
	
	aInfoOut.cameraMatrixBuffer.Destroy();
	aInfoOut.cameraVectorBuffer.Destroy();
	aInfoOut.instanceBuffer.Destroy();
	aInfoOut.lineVertexBuffer.Destroy();
}

void SceneRenderer::Init()
{
	Engine& engine = Engine::GetInstance();
	
    VkInit::DescriptorSetLayoutData input;
    input.count = 2;
    input.types.push_back(vk::DescriptorType::eUniformBuffer);
    input.types.push_back(vk::DescriptorType::eStorageBuffer);
    localBufferDescriptorPool = VkInit::MakeDescriptorPool(engine.GetLogicalDevice(), 1000, input);
}

void SceneRenderer::Destroy()
{
	Engine::GetInstance().GetLogicalDevice().destroyDescriptorPool(localBufferDescriptorPool);
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
		memcpy(aObjectInfo.cameraMatrixBuffer.GetWriteLocation(), &aObjectInfo.cameraMatrixData, sizeof(aObjectInfo.cameraMatrixData));
	}

	{
		memcpy(aObjectInfo.cameraVectorBuffer.GetWriteLocation(), &aObjectInfo.cameraVectorData, sizeof(aObjectInfo.cameraVectorData));
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
		void* dst = static_cast<char*>(aObjectInfo.instanceBuffer.GetWriteLocation()) + startInstance * sizeof(VkInit::ModelInstance);
		memcpy(dst, instanceMap.data()[i].data(), 
			batchSize * sizeof(VkInit::ModelInstance));
		startInstance += batchSize;
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
	
	aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubemapMaterial.GetPipelineLayout(), 0,
		aObjectInfo.cubemapDescriptorSet.GetDescriptorSet(), {});
	aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubemapMaterial.GetPipelineLayout(), 1, 
		Vulkan::TextureDescriptor::GetInstance().GetDescriptorSet(),{});

	cubemapMaterial.Bind(aCommandBuffer,aImageInfo.resolutionId);
	// myCubemap.Use(aCommandBuffer, myPipelineLayouts[VkInit::PipelineType::Cubemap]);
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
	
	aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Vulkan::GetPipelineLayout(Vulkan::RenderStageType::Forward), 0, 
		aObjectInfo.forwardDescriptorSet.GetDescriptorSet(),{});
	aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Vulkan::GetPipelineLayout(Vulkan::RenderStageType::Forward), 1, 
		Vulkan::TextureDescriptor::GetInstance().GetDescriptorSet(),{});

	uint32_t startInstance = 0;
	
	auto& materials = Wonder::GetMaterialRegistry().AccessAssetMap();
	for (uint32_t i = 0; i < instanceMap.size(); ++i)
	{
		const uint32_t instanceCount = static_cast<uint32_t>(instanceMap.data()[i].size());
		

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
		
		aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material.GetPipelineLayout(), 0,
		aObjectInfo.cubemapDescriptorSet.GetDescriptorSet(), {});
		aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material.GetPipelineLayout(), 1, 
			Vulkan::TextureDescriptor::GetInstance().GetDescriptorSet(),{});
		
		material.RemoveRenderImages();
		material.AddRenderImage(aImageInfo.intermediateImage.GetRenderImageId(engine.GetCurrentSwapchainIndex()));
		material.Bind(aCommandBuffer,aImageInfo.resolutionId);
	}
	else
	{
		aImageInfo.myCopyStage.StartRenderPass(aCommandBuffer,VkInit::GetRenderPass(Vulkan::RenderStageType::FullscreenOffscreen),
			Vulkan::GetAllResolutions().at(aImageInfo.resolutionId));
		
		auto& material = Wonder::GetMaterialRegistry().AccessAssetFromID(Wonder::FULLSCREEN_COPY_OFFSCREEN);
		
		aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material.GetPipelineLayout(), 0,
		aObjectInfo.cubemapDescriptorSet.GetDescriptorSet(), {});
		aCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material.GetPipelineLayout(), 1, 
			Vulkan::TextureDescriptor::GetInstance().GetDescriptorSet(),{});
		
		material.RemoveRenderImages();
		material.AddRenderImage(aImageInfo.intermediateImage.GetRenderImageId(engine.GetCurrentSwapchainIndex()));
		material.Bind(aCommandBuffer,aImageInfo.resolutionId);
	}
	
	aCommandBuffer.draw(6, 1, 0, 0);
	aCommandBuffer.endRenderPass();
}

