#include "EditorViewport.h"

#include <Engine.h>

void Wonder::EditorViewport::ResizeCheck()
{
    ImVec2 imageSize = ImGui::GetContentRegionAvail();
    if (imageSize != myViewportSize)
    {
        ResizeViewport(static_cast<uint32_t>(imageSize.x),static_cast<uint32_t>(imageSize.y));
    }
}

void Wonder::EditorViewport::RenderViewport(const SceneRenderer::SceneObjectInfo& aSceneObjectInfo)
{
    Engine& engine = Engine::GetInstance();
    myRenderImage.TransitionToTarget(engine.GetCurrentCommandBuffer());
        
    SceneRenderer::Render(engine.GetCurrentCommandBuffer(),mySceneImageInfo,aSceneObjectInfo);
    myRenderImage.TransitionToResource(engine.GetCurrentCommandBuffer());
        
    ImGui::Image(myRenderImage.GetImGuiDescriptorSet(engine.GetCurrentSwapchainIndex()),myViewportSize);
}

void Wonder::EditorViewport::Destroy()
{
    if (myRenderImage.IsInitialized())
    {
        myRenderImage.Destroy();
        SceneRenderer::DestroySceneImageInfo(mySceneImageInfo);
        
        Vulkan::DiscardResolution(myResolutionId);
    }
    
}

ImVec2 Wonder::EditorViewport::GetViewportSize() const
{
    return myViewportSize;
}

Vulkan::ResolutionId Wonder::EditorViewport::GetResolutionId() const
{
    return myResolutionId;
}

void Wonder::EditorViewport::ResizeViewport(uint32_t aWidth, uint32_t aHeight)
{
    Engine::GetInstance().WaitForIdle();
    
    if (myResolutionId == Vulkan::INVALID_RESOLUTION_ID)
    {
        myResolutionId = Vulkan::GetOrCreateResolution();
    }
    Vulkan::SetResolution(myResolutionId,vk::Extent2D(aWidth,aHeight));
    
    if (myRenderImage.IsInitialized())
    {
        myRenderImage.Destroy();
        SceneRenderer::DestroySceneImageInfo(mySceneImageInfo);
    }
    
    myRenderImage.Init(aWidth, aHeight,vk::Format::eB8G8R8A8Unorm,false);
    myRenderImage.CreateImGuiDescriptorSets();
    SceneRenderer::CreateSceneImageInfo(mySceneImageInfo,myRenderImage,vk::Extent2D(aWidth,aHeight));
    mySceneImageInfo.resolutionId = myResolutionId;
    
    myViewportSize.x = static_cast<float>(aWidth);
    myViewportSize.y = static_cast<float>(aHeight);
}
