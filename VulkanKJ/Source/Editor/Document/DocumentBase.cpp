#include "DocumentBase.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "EditorGlobals.h"
#include "AssetBrowser/ImGuiWidgetsEditor.h"

namespace Wonder
{
    Tga::StringId DocumentBase::GetImGuiName() const
    {
        return myImGuiName;
    }

    bool DocumentBase::WasVisibleLastFrame() const
    {
        return myWasVisibleLastFrame;
    }

    bool DocumentBase::ShouldClose() const
    {
        return myShouldClose;
    }

    void DocumentBase::GenerateDocks()
    {
        ImGui::DockBuilderSetNodeSize(globalDocumentDockingSpaceID, globalDocumentDockSize);
        ImGui::DockBuilderDockWindow(myImGuiName.GetString(), globalDocumentDockingSpaceID);
        ImGui::DockBuilderFinish(globalDocumentDockingSpaceID);
        

        Wonder::DockFlagOverride(ImGuiDockNodeFlags_NoWindowMenuButton);
        ImGui::Begin(myImGuiName.GetString());

        GenerateSubDocks();

        ImGui::End();
    }

    void DocumentBase::BaseUpdate()
    {
        int mainWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        if (myCommandManager.IsSameAsLastSave() == false)
        {
            mainWindowFlags |= ImGuiWindowFlags_UnsavedDocument;
        }

        bool open = true;

        ImGui::Begin(myImGuiName.GetString(), &open, mainWindowFlags);

        const ImGuiID imGuiID     = ImGui::GetID(myImGuiName.GetString());
        const ImVec2 docSpaceSize = ImGui::GetContentRegionAvail();
        ImGui::DockSpace(imGuiID, docSpaceSize, ImGuiDockNodeFlags_None, &globalDocumentLevelWindowClass);
        myWasVisibleLastFrame = myIsVisible;
        myIsVisible           = ImGui::GetCurrentWindow()->DockTabIsVisible;

        ImGui::End();

        if (open == false)
        {
            if (myCommandManager.IsSameAsLastSave())
            {
                myShouldClose = true;
            }
            else if (ImGui::IsPopupOpen(CLOSE_DOCUMENT_POPUP) == false)
            {
                ImGui::OpenPopup(CLOSE_DOCUMENT_POPUP);
                myShowClosePopup = true;
            }
        }
        if (myShowClosePopup)
        {
            DisplayClosePopUp();
        }
    }

    bool DocumentBase::IsVisible() const
    {
        return myIsVisible;
    }

    void DocumentBase::DisplayClosePopUp()
    {
        if (ImGui::BeginPopupModal(CLOSE_DOCUMENT_POPUP, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you sure about that?");

            ImGui::Separator();

            if (ImGui::Button("Yes", ImVec2(120, 0)))
            {
                myShouldClose = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("No", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                myShowClosePopup = false;
            }

            ImGui::EndPopup();
        }
    }

    void DocumentBase::SetPanelInfo(int& aInfo) const
    {
        aInfo = 0;
        if (ImGui::IsWindowHovered())
        {
            aInfo |= PanelFlag_Hovered;
        }
        if (ImGui::IsWindowContentHoverable(ImGui::GetCurrentWindow()))
        {
            aInfo |= PanelFlag_Hoverable;
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_DockHierarchy))
        {
            aInfo |= PanelFlag_Focused;
        }

        if (ImGui::GetCurrentWindow()->DockTabIsVisible)
        {
            aInfo |= PanelFlag_Visible;
        }
    }
}


