#pragma once
#include <imgui.h>
#include <Rendering/SceneRenderer.h>

namespace Wonder
{
    class EditorViewport
    {
    public:
        void ResizeCheck();
        void RenderViewport(const SceneRenderer::SceneObjectInfo& aSceneObjectInfo);
    
        void Destroy();
        
        ImVec2 GetViewportSize() const;
        Vulkan::ResolutionId GetResolutionId() const;
    private:
        void ResizeViewport(uint32_t aWidth, uint32_t aHeight);
    
        SceneRenderer::SceneImageInfo mySceneImageInfo;
        RenderImage myRenderImage;
        ImVec2 myViewportSize;
        Vulkan::ResolutionId myResolutionId{};
    };
}

