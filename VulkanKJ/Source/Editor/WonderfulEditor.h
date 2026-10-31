#pragma once
#include <Util/FileManagement/FileTypes.h>

#include "Document/DocumentBase.h"

class WonderfulEditor
{
public:
    void Init();
    void Update();
    void Destroy() const;
    
    void OpenDocument(const Filepath& aFilepath);
    void CreateAsset(const Filepath& aFilepath, const char* aExtension) const;
    
    static WonderfulEditor& GetInstance();
private:
    void GenerateDocks();
    void DisplayTopBar() const;
    
    void MakeFileExtensionFunctions() const;
    void ImportFile(const Filepath& aFilepath) const;
    
    std::vector<std::shared_ptr<Wonder::DocumentBase>> myDocuments;
    bool myDockIsInitialized{};
};
