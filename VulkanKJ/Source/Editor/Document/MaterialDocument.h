#pragma once
#include <AssetManagement/MaterialAsset.h>
#include <Input/CameraController.h>
#include <Rendering/SceneRenderer.h>

#include "DocumentBase.h"
#include "Util/EditorViewport.h"
#include "Util/LineGrid.h"

namespace Wonder
{
    class MaterialDocument final : public DocumentBase
    {
        enum class PanelNames
        {
            Viewport,
            Properties,
            Count,
        };
        
    public:
        void Init(const Filepath& aFilePath) override;
        void Update(float aDeltaTime) override;
        void Close() override;
        void Save() override;

        void CommandUpdateMaterial(const MaterialAsset& aMaterial);
    protected:
        void GenerateSubDocks() override;
        
        void DisplayProperties();
        
        void RenderMaterial();
        
        MaterialAsset& AccessEditMaterial() const;
        
        SceneRenderer::SceneObjectInfo mySceneObjectInfo;
        EditorViewport myEditorViewport;
        
        CameraController myCameraController;
        LineGrid myLineGrid;
        
        MaterialAsset myReferenceMaterial;
        
        MaterialId myEditMaterialId;
        MaterialId mySavedMaterialId;

        ImGuiWindowFlags myWindowFlags{};
    };
}


