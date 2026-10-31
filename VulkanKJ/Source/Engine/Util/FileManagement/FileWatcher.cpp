#include "FileWatcher.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_set>

#include "Engine.h"
#include "FileFunctions.h"
#include "FileTypes.h"
#include "Settings.h"
#include "AssetManagement/Loaders/MaterialBuildQueue.h"
#include "Shaders/ShaderCompilation.h"
#include "Shaders/Shaders.h"
#include "Util/Logging/Logging.h"

namespace Wonder::FileWatcher
{
    static std::atomic<bool> localShouldTerminate = false;
    static std::atomic<bool> localHasPendingFileUpdates = false;
    
    static std::thread localFileWatcherThread;
    
    static std::mutex localFileWatcherMutex;
    static std::condition_variable localFileWatcherWaitCondition;
    
    static std::unordered_map<Filepath,long long> localCurrentTimeStamps;
    static std::vector<Filepath> localShadersToRecompile;
    static std::unordered_set<Filepath> localShaderExtensions{".vert", ".frag"};
    
    static void CheckShaderFiles()
    {
        Filepath shaderFolderPath = Wonder::GetSourcePath() / "Engine/Shaders";
        const Filepath spvDirectory = Wonder::GetBinPath() / "Shaders";
        
        for (auto& item : std::filesystem::recursive_directory_iterator(shaderFolderPath))
        {
            Filepath extension = item.path().extension();
            
            if (localShaderExtensions.contains(extension) == false)
            {
                continue;
            }
            
            Filepath spvFileName = Vulkan::Internal::GetSpvFilename(item.path().filename());
            Filepath spvFilePath = spvDirectory / spvFileName;
            
            long long shaderTimeStamp = FileFunctions::GetFileTimeStamp(item.path());
            
            auto it = localCurrentTimeStamps.find(spvFileName);
            
            if (it != localCurrentTimeStamps.end())
            {
                if (shaderTimeStamp > it->second)
                {
                    localShadersToRecompile.emplace_back(item.path());
                    localCurrentTimeStamps.insert_or_assign(spvFileName,shaderTimeStamp);
                }
                continue;
            }
            localCurrentTimeStamps.insert({spvFileName,shaderTimeStamp});
            
            if (exists(spvFilePath) == false)
            {
                localShadersToRecompile.emplace_back(item.path());
                continue;
            }
            
            
            
            if (shaderTimeStamp > FileFunctions::GetFileTimeStamp(spvFilePath))
            {
                localShadersToRecompile.emplace_back(item.path());
            }
        }
    }
    
    static void ThreadLoop()
    {
        while (localShouldTerminate == false)
        {
            std::unique_lock<std::mutex> fileWatcherLock(localFileWatcherMutex);
            localFileWatcherWaitCondition.wait(fileWatcherLock, [&]
            {
                return localShouldTerminate || (localHasPendingFileUpdates == false);
            });
            
            if (localShouldTerminate)
            {
                fileWatcherLock.unlock();
                return;
            }
            
            CheckShaderFiles();
            
            fileWatcherLock.unlock();
            if (localShadersToRecompile.empty() == false)
            {
                localHasPendingFileUpdates = true;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    
    
    
    void Init()
    {
        localFileWatcherThread = std::thread(ThreadLoop);
    }

    void CheckForUpdates()
    {
        if (localHasPendingFileUpdates == false)
        {
            return;
        }
        
        Engine::GetInstance().WaitForIdle();
        
        std::unique_lock<std::mutex> fileWatcherLock(localFileWatcherMutex);

        const Filepath spvDirectory = Wonder::GetBinPath() / "Shaders";
        
        auto& materials = GetMaterialRegistry().AccessAssetMap();
        
        for (auto& shaderPath : localShadersToRecompile)
        {
            Filepath shaderFileName = shaderPath.filename();
            LOG_INFO("Shader %s needs to be recompiled, re-compiling...", shaderFileName.string().c_str());
            
            Filepath spvFilePath = spvDirectory / Vulkan::Internal::GetSpvFilename(shaderFileName);
            
            if (Vulkan::CompileShader(shaderPath,spvFilePath,Vulkan::GetShaderKind(shaderFileName)))
            {
                Vulkan::ReloadModule(shaderFileName,Engine::GetInstance().GetLogicalDevice(),Vulkan::GetShaderStage(shaderFileName.extension()));
                Tga::StringId shaderId = Tga::StringRegistry::RegisterOrGetString(shaderFileName.string());

                for (uint32_t i = 0; i < materials.size(); i++)
                {
                    auto& material = materials.data()[i];
                    if (material.AccessVertexShaderName() == shaderId || material.AccessFragmentShaderName() == shaderId)
                    {
                        material.AdjustPushConstantsToReflection();
                        MaterialBuildQueue::AddMaterialToQueue(materials.dataKeys()[i]);
                    }
                }
            }
        }
        
        localShadersToRecompile.clear();
        
        localHasPendingFileUpdates = false;
        
        localFileWatcherWaitCondition.notify_all();
    }

    void Destroy()
    {
        localShouldTerminate = true;
        localFileWatcherThread.join();
    }
}
