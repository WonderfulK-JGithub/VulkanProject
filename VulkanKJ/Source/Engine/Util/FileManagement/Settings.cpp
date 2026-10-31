#include "Settings.h"

#include <nlohmann/json.hpp>

#include <Windows.h>
#include "FileFunctions.h"
#include "Util/Logging/Logging.h"

namespace Wonder
{
    static Filepath localBinPath;
    static Filepath localAssetsPath;
    static Filepath localDataPath;
    static Filepath localEngineAssetPath;
    static Filepath localIntermediateAssetPath;
    
    static Filepath localSourcePath;
    
    
    
    void LoadSettings()
    {
        WCHAR executablePathWString[MAX_PATH]{ 0 };
        // Game settings
        if (!GetModuleFileName(NULL, executablePathWString, sizeof(executablePathWString))) 
        {
            assert(false && "GetModuleFileName failed in Tga::LoadSettings");
        }

        std::filesystem::path executablePath = Filepath(executablePathWString).parent_path();
        
        nlohmann::json settingsJson;
        FileFunctions::LoadFileToJson(executablePath / "Settings/Wonderful_Engine.json",settingsJson);
        
        const nlohmann::json& pathsJson = settingsJson["assets_path"];
        
        localAssetsPath = (executablePath / Filepath(pathsJson["assets"].get<std::string>())).lexically_normal();
        
        LOG_DEBUG("Assets Folder path: %s", localAssetsPath.string().c_str());
        
        localBinPath = (executablePath / Filepath(pathsJson["bin"].get<std::string>())).lexically_normal();
        
        LOG_DEBUG("Bin Folder path: %s", localBinPath.string().c_str());
        
        localSourcePath = (executablePath / Filepath(pathsJson["source"].get<std::string>())).lexically_normal();
        
        LOG_DEBUG("Source Folder path: %s", localSourcePath.string().c_str());
        
        localDataPath = (executablePath / Filepath(pathsJson["data"].get<std::string>())).lexically_normal();
        
        LOG_DEBUG("data Folder path: %s", localDataPath.string().c_str());
        
        localEngineAssetPath = (executablePath / Filepath(pathsJson["engine_assets"].get<std::string>())).lexically_normal();
        
        LOG_DEBUG("Engine Asset Folder path: %s", localEngineAssetPath.string().c_str());
        
        localIntermediateAssetPath = (executablePath / Filepath(pathsJson["intermediateAssets"].get<std::string>())).lexically_normal();
        
        LOG_DEBUG("Intermediate Asset Folder path: %s", localIntermediateAssetPath.string().c_str());
    }

    const Filepath& GetBinPath()
    {
        return localBinPath;
    }

    const Filepath& GetMainAssetsPath()
    {
        return localAssetsPath;
    }

    const Filepath& GetEngineAssetPath()
    {
        return localEngineAssetPath;
    }

    const Filepath& GetIntermediateAssetPath()
    {
        return localIntermediateAssetPath;
    }

    const Filepath& GetSourcePath()
    {
        return localSourcePath;
    }

    const Filepath& GetDataPath()
    {
        return localDataPath;
    }
}
