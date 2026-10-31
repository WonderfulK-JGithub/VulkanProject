#pragma once
#include "FileTypes.h"

namespace Wonder
{
    void LoadSettings();
    
    const Filepath& GetBinPath();
    const Filepath& GetMainAssetsPath();
    const Filepath& GetEngineAssetPath();
    const Filepath& GetIntermediateAssetPath();
    const Filepath& GetDataPath();
    
    const Filepath& GetSourcePath();
    
    
    
}
