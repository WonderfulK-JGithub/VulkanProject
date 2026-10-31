#pragma once
#include <Command/CommandManager.h>
#include <Util/StringRegistry.h>
#include <Util/FileManagement/FileTypes.h>
#include <imgui/imgui.h>

namespace Wonder
{
    enum PanelFlag
    {
        PanelFlag_Hovered = 1,
        PanelFlag_Focused = 1 << 1,
        PanelFlag_Visible = 1 << 2,
        PanelFlag_Hoverable = 1 << 3,
    };
    
    class DocumentBase
    {
    public:
        virtual ~DocumentBase() = default;
        virtual void Init(const Filepath& aFilePath) = 0;
        virtual void Update(float aDeltaTime) = 0;
        virtual void Close() = 0;
        virtual void Save() = 0;
    
        Tga::StringId GetImGuiName() const;
        bool WasVisibleLastFrame() const;
        bool ShouldClose() const;
    
        void GenerateDocks();
    protected:
        virtual void GenerateSubDocks() = 0;
    
        void BaseUpdate();

        bool IsVisible() const;

    
        void DisplayClosePopUp();
        void SetPanelInfo(int& aInfo) const;

        static constexpr const char* CLOSE_DOCUMENT_POPUP = "Close document?";
    
        CommandManager myCommandManager;
        
        std::vector<std::string> myPanelNames;
        std::vector<int> myPanelInfos;

        Tga::StringId myImGuiName;
        Filepath myFilePath;

        bool myIsVisible           = false;
        bool myWasVisibleLastFrame = true;
        bool myShouldClose         = false;
        bool myShowClosePopup      = false;
    };
}


