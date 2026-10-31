#pragma once
#include <Util/FileManagement/FileTypes.h>

namespace AssetBrowser
{
    void Init();
    void Display();
    void Destroy();

    void GeneratePathCache();
    Filepath& GetCurrentDirectory();
    namespace Internal
    {
        void GenerateDockSpace();
        
        void DisplayFiles();
        
        void DisplayDirectoryOrder();
        void SetNewDirectory(const Filepath& aDirectoryPath);
        
        void DisplayNamingPopup();
    }
}
