#pragma once
#include <vector>

#include "RenderImage.h"
#include "Vulkan/RenderStructs.h"

class RenderStage
{
public:
    
    void AddRenderImage(const RenderImage& aRenderImage);
    
    void CreateFrameBuffers(Vulkan::RenderStageType aType,vk::Extent2D aFrameExtent);
    void Destroy();
    
    void StartRenderPass(vk::CommandBuffer aCommandBuffer,vk::RenderPass aRenderPass,vk::Extent2D aRenderExtent) const;
    
private:
    std::vector<const RenderImage*> myRenderImages;
    
    std::vector<vk::Framebuffer> myFrameBuffers;
};
