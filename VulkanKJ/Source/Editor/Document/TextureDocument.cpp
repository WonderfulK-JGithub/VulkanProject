#include "TextureDocument.h"

#include <imgui_internal.h>

#include "EditorGlobals.h"

void Wonder::TextureDocument::Init(const Filepath& aFilePath)
{
    const std::string fileName = aFilePath.filename().string();
    
    myImGuiName = Tga::StringRegistry::RegisterOrGetString(fileName);
    
    myPanelNames.resize(static_cast<uint32_t>(PanelNames::Count));
    myPanelInfos.resize(myPanelNames.size());
    
    myPanelNames[static_cast<uint32_t>(PanelNames::Image)] = "Image##Document" + fileName;
    
    myWindowFlags = ImGuiWindowFlags_NoMove;
}

void Wonder::TextureDocument::Update([[maybe_unused]] float aDeltaTime)
{
    BaseUpdate();
    
    if (myWasVisibleLastFrame || true)
    {
        ImGui::Begin(myPanelNames[static_cast<uint32_t>(PanelNames::Image)].c_str(),nullptr,myWindowFlags);
        ImGui::Text("aa");
        ImGui::End();
    }
}

void Wonder::TextureDocument::Close()
{
}

void Wonder::TextureDocument::Save()
{
}

void Wonder::TextureDocument::GenerateSubDocks()
{
    const ImGuiID imGuiID     = ImGui::GetID(myImGuiName.GetString());
    const ImVec2 docSpaceSize = ImGui::GetContentRegionAvail();
    ImGui::DockSpace(imGuiID, docSpaceSize, ImGuiDockNodeFlags_None, &globalDocumentLevelWindowClass);

    const ImGuiID dockSpaceId = imGuiID;

    ImGui::DockBuilderRemoveNode(dockSpaceId);
    ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceId, docSpaceSize);

    ImGuiID center = dockSpaceId,right = 0;
    
    ImGui::DockBuilderSplitNode(center,ImGuiDir_Right,0.3f, &right,&center);

    ImGui::DockBuilderDockWindow(myPanelNames[static_cast<uint32_t>(PanelNames::Image)].c_str(), center);

    ImGui::DockBuilderFinish(dockSpaceId);
}
