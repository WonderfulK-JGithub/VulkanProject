#pragma once
#include <ECS/Reflection/ComponentReflectionUtil.h>
#include <Input/CameraController.h>
#include <Rendering/SceneRenderer.h>

#include "DocumentBase.h"
#include "Util/EditorViewport.h"
#include "Util/LineGrid.h"

namespace Wonder
{
    class ModelDocument final : public DocumentBase
    {
        enum class PanelNames
        {
            Image,
            Count,
        };
        
    public:
        void Init(const Filepath& aFilePath) override;
        void Update(float aDeltaTime) override;
        void Close() override;
        void Save() override;

    protected:
        void GenerateSubDocks() override;
        
        void PrepareRenderInfo();
        
        SceneRenderer::SceneObjectInfo mySceneObjectInfo;
        EditorViewport myEditorViewport;
        
        CameraController myCameraController;
        
        ImGuiWindowFlags myWindowFlags{};
        LineGrid myLineGrid;
        
        ModelId myModelId;
    };
}


