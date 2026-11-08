#pragma once
#include <AssetManagement/PrefabAsset.h>
#include <ECS/ECSInstance.h>
#include <ECS/Entity.h>
#include <Input/CameraController.h>
#include <Rendering/SceneRenderer.h>

#include "DocumentBase.h"
#include "Util/EditorViewport.h"
#include "Util/LineGrid.h"

namespace Wonder
{
    class PrefabDocument final : public DocumentBase
    {
        enum class PanelNames
        {
            Hierarchy,
            Viewport,
            Properties,
            Count,
        };
        
    public:
        void Init(const Filepath& aFilePath) override;
        void Update(float aDeltaTime) override;
        void Close() override;
        void Save() override;

        void CommandUpdatePrefab(const PrefabAsset& aPrefabAsset);
    private:
        void MakeUpdateCommand();
        
        void GenerateSubDocks() override;
        
        void DisplayHierarchy();
        void DisplayHierarchyObject(PrefabObjectId aPrefabObjectId);
        
        void DisplayProperties();
        void DisplayAddComponent();
        
        void RecreatePrefab() const;
        void PrepareRenderInfo();
        
        SceneRenderer::SceneObjectInfo mySceneObjectInfo;
        EditorViewport myEditorViewport;
        
        CameraController myCameraController;
        LineGrid myLineGrid;
        
        ECS::ECSInstance myEcs;
        PrefabAsset myReferencePrefab;
        PrefabAsset myEditPrefab;
        
        ECS::EntityReference mySelection{};
        PrefabId myId;
        
        ECS::EntityID myPrefabHeadEntityId{123};
        
        ImGuiWindowFlags myWindowFlags{};
    
    };
}


