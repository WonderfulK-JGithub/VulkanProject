#pragma once
#include "ModelInstanceContainer.h"
#include "RenderImage.h"
#include "RenderStage.h"
#include "Vulkan/Buffer/VulkanBuffer.h"
#include "Vulkan/VkInit/Descriptors.h"

namespace SceneRenderer
{
    constexpr uint32_t MAX_LINE_COUNT = 50000;
    
    struct SceneImageInfo
    {
        RenderImage* finalRenderImage{};
        
        RenderImage intermediateImage;
        RenderImage depthImage;
        
        RenderStage myCubemapStage;
        RenderStage myForwardStage;
        RenderStage myCopyStage;
        
        Vulkan::ResolutionId resolutionId{};
    };
    
    struct SceneObjectInfo
    {
        Wonder::ModelInstanceContainer instances;
        std::vector<Vulkan::LineVertex> linePoints;
        
        VkInit::CameraMatrices cameraMatrixData;
        VkInit::CameraVectors cameraVectorData;
        
        Wonder::MaterialId cubemapMaterial;
        
        Vulkan::VulkanBuffer cameraMatrixBuffer;
        Vulkan::VulkanBuffer cameraVectorBuffer;
        Vulkan::VulkanBuffer instanceBuffer;
        Vulkan::VulkanBuffer lineVertexBuffer;
	
        VkInit::VulkanDescriptorSet cubemapDescriptorSet;
        VkInit::VulkanDescriptorSet forwardDescriptorSet;
        VkInit::DescriptorWriter descriptorWriter;
    };
    
    void CreateSceneImageInfo(SceneImageInfo& aInfoOut,RenderImage& aFinalRenderImage,vk::Extent2D aSize);
    void CreateSceneObjectInfo(SceneObjectInfo& aInfoOut);
    
    void DestroySceneImageInfo(SceneImageInfo& aInfoOut);
    void DestroySceneObjectInfo(SceneObjectInfo& aInfoOut);
    
    void Init();
    void Destroy();
    
    void Render(vk::CommandBuffer aCommandBuffer, SceneImageInfo& aImageInfo,const SceneObjectInfo& aObjectInfo);

    namespace Internal
    {
        void Prepare(vk::CommandBuffer aCommandBuffer,const SceneObjectInfo& aObjectInfo);
        
        void RenderCubemap(vk::CommandBuffer aCommandBuffer,SceneImageInfo& aImageInfo,const SceneObjectInfo& aObjectInfo);
        void RenderForward(vk::CommandBuffer aCommandBuffer,SceneImageInfo& aImageInfo,const SceneObjectInfo& aObjectInfo);
        void CopyToFinalImage(vk::CommandBuffer aCommandBuffer,SceneImageInfo& aImageInfo,const SceneObjectInfo& aObjectInfo);
    }
}
