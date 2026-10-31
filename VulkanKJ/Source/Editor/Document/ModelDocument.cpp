#include "ModelDocument.h"

#include <Engine.h>
#include <imgui_internal.h>
#include <AssetManagement/TextureAsset.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "EditorGlobals.h"

void Wonder::ModelDocument::Init(const Filepath& aFilePath)
{
    const std::string fileName = aFilePath.filename().replace_extension().string();
    
    myImGuiName = Tga::StringRegistry::RegisterOrGetString(fileName);
    
    myPanelNames.resize(static_cast<uint32_t>(PanelNames::Count));
    myPanelInfos.resize(myPanelNames.size());
    
    myPanelNames[static_cast<uint32_t>(PanelNames::Image)] = "Image##Document" + fileName;
    
    myWindowFlags = ImGuiWindowFlags_NoMove;
    
    myModelId = GetModelRegistry().GetIDFromName(fileName.c_str());
    
    SceneRenderer::CreateSceneObjectInfo(mySceneObjectInfo);
    mySceneObjectInfo.cubemapMaterial = TEST_CUBEMAP_MATERIAL;
    
    mySceneObjectInfo.instances.AddInstance(Wonder::TEST_MATERIAL_1, 
                    Wonder::GetModelRegistry().AccessAssetFromID(myModelId).GetMeshes().front().id,
                    {glm::mat4(1.f)});
    
    myCameraController.SetPosition(glm::vec3(0.0f, 1.0f, 5.0f));
}

void Wonder::ModelDocument::Update([[maybe_unused]] float aDeltaTime)
{
    BaseUpdate();
    
    if (myIsVisible == false)
    {
        return;
    }
    myCameraController.Update();
    myLineGrid.MakeLines(glm::xyz(myCameraController.GetTransform()[3]),10);
    
    ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Image)].c_str(),nullptr,myWindowFlags);
    SetPanelInfo(myPanelInfos[static_cast<uint32_t>(PanelNames::Image)]);
        
    myEditorViewport.ResizeCheck();
    myCameraController.SetProjection(myEditorViewport.GetViewportSize().x,myEditorViewport.GetViewportSize().y);
    PrepareRenderInfo();
    myEditorViewport.RenderViewport(mySceneObjectInfo);
    
    ImGui::End();
}

void Wonder::ModelDocument::Close()
{
    Engine::GetInstance().WaitForIdle();
    
    myEditorViewport.Destroy();
    SceneRenderer::DestroySceneObjectInfo(mySceneObjectInfo);
}

void Wonder::ModelDocument::Save()
{
}

void Wonder::ModelDocument::GenerateSubDocks()
{
    const ImGuiID imGuiID     = ImGui::GetID(myImGuiName.GetString());
    const ImVec2 docSpaceSize = ImGui::GetContentRegionAvail();
    ImGui::DockSpace(imGuiID, docSpaceSize, ImGuiDockNodeFlags_None, &globalDocumentLevelWindowClass);

    const ImGuiID dockSpaceId = imGuiID;

    ImGui::DockBuilderRemoveNode(dockSpaceId);
    ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceId, docSpaceSize);

    ImGuiID center = dockSpaceId;

    ImGui::DockBuilderDockWindow(myPanelNames[static_cast<uint32_t>(PanelNames::Image)].c_str(), center);

    ImGui::DockBuilderFinish(dockSpaceId);
}

void Wonder::ModelDocument::PrepareRenderInfo()
{
    const glm::mat4 view = myCameraController.GetView();
    
    glm::mat4 projection = myCameraController.GetProjection();
    
    mySceneObjectInfo.cameraMatrixData.view = view;
    mySceneObjectInfo.cameraMatrixData.projection = projection;
    mySceneObjectInfo.cameraMatrixData.viewProjection = projection * view;

    mySceneObjectInfo.cameraVectorData.right = myCameraController.GetTransform()[0];
    mySceneObjectInfo.cameraVectorData.up = myCameraController.GetTransform()[1];
    mySceneObjectInfo.cameraVectorData.forward = -myCameraController.GetTransform()[2];
    
    mySceneObjectInfo.linePoints = myLineGrid.GetLinePoints();
}
