#include "Shaders.h"

#include <fstream>
#include <unordered_map>

#include "Util/FileManagement/Settings.h"
#include "Util/Logging/Logging.h"
#include <shaderc/shaderc.hpp>

#include "imgui.h"
#include "ShaderCompilation.h"
#include "ShaderReflection.h"
#include "Util/DebugUtilities.h"
#include "Util/ImGuiUtil.h"

namespace Vulkan
{
    std::unordered_map<std::string, Shader> localLoadedShaders;
    
    Goose::MappedVector<Tga::StringId, Filepath> localAllShaderNamesAndPaths;
    Goose::MappedVector<Tga::StringId, Filepath> localVertexNamesAndPaths;
    Goose::MappedVector<Tga::StringId, Filepath> localFragmentNamesAndPaths;
    
    bool localHasLoadedNames{};
}

std::vector<char> Vulkan::Internal::ReadFile(const Filepath& aFilepath)
{
    std::ifstream file(aFilepath, std::ios::ate | std::ios::binary);

    if (file.is_open() == false)
    {
        LOG_ERROR("Failed to open file %s", aFilepath.string().c_str());
        return {};
    }

    size_t fileSize{ static_cast<size_t>(file.tellg()) };

    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return buffer;
}

vk::ShaderModule Vulkan::Internal::CreateShaderModule(const Filepath& aFilepath, vk::Device aDevice)
{
    std::vector<char> sourceCode = ReadFile(aFilepath);

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.flags = vk::ShaderModuleCreateFlags();
    createInfo.codeSize = sourceCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

    try 
    {
        return aDevice.createShaderModule(createInfo);
    }
    catch (vk::SystemError err)
    {
        LOG_ERROR("Failed to create shader module for %s, ", aFilepath.string().c_str(),err.what());
    }

    return vk::ShaderModule();
}

vk::PipelineShaderStageCreateInfo MakeShaderInfo(const vk::ShaderModule& aShaderModule, const vk::ShaderStageFlagBits& aStage)
{
    vk::PipelineShaderStageCreateInfo shaderInfo{};
    shaderInfo.flags == vk::PipelineShaderStageCreateFlags();
    shaderInfo.stage = aStage;
    shaderInfo.module = aShaderModule;
    shaderInfo.pName = "main";

    return shaderInfo;
}

Goose::MappedVector<Tga::StringId, Filepath>& Vulkan::GetAllVertexNamesAndPaths()
{
    if (localHasLoadedNames == false)
    {
        Internal::LoadAllShaderNamesAndPaths();
    }
    
    return localVertexNamesAndPaths;
}

Goose::MappedVector<Tga::StringId, Filepath>& Vulkan::GetAllFragmentNamesAndPaths()
{
    if (localHasLoadedNames == false)
    {
        Internal::LoadAllShaderNamesAndPaths();
    }
    
    return localFragmentNamesAndPaths;
}

vk::PipelineShaderStageCreateInfo Vulkan::GetShaderInfo(const std::string& aFilename, vk::Device aDevice, vk::ShaderStageFlagBits aStage)
{
    if (localLoadedShaders.contains(aFilename))
    {
        return localLoadedShaders.at(aFilename).shaderInfo;
    }

    Filepath spvFileName = Internal::GetSpvFilename(aFilename);
    
    const Filepath spvDirectory = Wonder::GetBinPath() / "Shaders";
    
    if (exists(spvDirectory / spvFileName) == false)
    {
        if (Internal::TryCompileShader(aFilename) == false)
        {
            LOG_ERROR("Cannot create shader info because shader compilation failed");
            return {};
        }
    }
    
    Shader newShader;
    newShader.module = Internal::CreateShaderModule(spvDirectory / spvFileName, aDevice);
    newShader.type = aStage;
    newShader.shaderInfo = MakeShaderInfo(newShader.module, aStage);

    localLoadedShaders.insert({ aFilename,newShader });

    return newShader.shaderInfo;
}

void Vulkan::DestroyAllModules(vk::Device aDevice)
{
    for (const auto& [key,shader] : localLoadedShaders)
    {
        aDevice.destroyShaderModule(shader.module);
    }
}

void Vulkan::DisplayDebugTap()
{
    if (DebugUtilities::BeginDebugTabItem("Shaders"))
    {
        if (ImGui::Button("Compile all shaders"))
        {
            Filepath shaderFolderPath = Wonder::GetSourcePath() / "Engine/Shaders";
            Filepath spvDirectory = Wonder::GetBinPath() / "Shaders";
            
            if (exists(spvDirectory) == false)
            {
                std::filesystem::create_directory(spvDirectory);
            }
            
            for (auto& item : std::filesystem::recursive_directory_iterator(shaderFolderPath))
            {
                const Filepath extension = item.path().extension();
                Filepath targetFileName = item.path().filename().replace_extension();
                
                shaderc_shader_kind shaderKind{};
                if (extension == ".vert")
                {
                    targetFileName += "_V";
                    shaderKind = shaderc_shader_kind::shaderc_glsl_default_vertex_shader;
                }
                else if (extension == ".frag")
                {
                    targetFileName += "_F";
                    shaderKind = shaderc_shader_kind::shaderc_glsl_default_fragment_shader;
                }
                else
                {
                    continue;
                }
                
                targetFileName.replace_extension(".spv");
                
                CompileShader(item.path().string(),(spvDirectory / targetFileName).string(),shaderKind);
            }
        }
        
        Wonder::ImGuiUtil::ThickSeparator(3.f);
        
        static Tga::StringId selectedShader = "None"_tgaid;
        static Goose::String32 nameFilter;
        Wonder::ImGuiUtil::DisplayComboFiltered("##SelectShader",localAllShaderNamesAndPaths.dataKeys(),
            static_cast<uint32_t>(localAllShaderNamesAndPaths.size()),selectedShader,nameFilter);
        
        if (ImGui::Button("Reflect"))
        {
            ShaderReflection::GetShaderPushConstantTypes(localAllShaderNamesAndPaths.at(selectedShader));
        }
    }
    DebugUtilities::EndDebugTabItem();
}

bool Vulkan::Internal::TryCompileShader(const Filepath& aFilename)
{
    Filepath shaderFolderPath = Wonder::GetSourcePath() / "Engine/Shaders";
    Filepath spvDirectory = Wonder::GetBinPath() / "Shaders";
    
    if (localHasLoadedNames == false)
    {
        LoadAllShaderNamesAndPaths();
    }
    
    Tga::StringId nameId = Tga::StringRegistry::RegisterOrGetString(aFilename.string());
    
    if (localAllShaderNamesAndPaths.contains(nameId) == false)
    {
        return false;
    }
    
    Filepath targetFileName = GetSpvFilename(aFilename);
    shaderc_shader_kind shaderKind = GetShaderKind(aFilename);
            
    return CompileShader(localAllShaderNamesAndPaths.at(nameId).string(),(spvDirectory / targetFileName).string(),shaderKind);
}

Filepath Vulkan::Internal::GetSpvFilename(const Filepath& aFilename)
{
    const Filepath extension = aFilename.extension();
    Filepath spvFileName = aFilename.filename().replace_extension();
    
    if (extension == ".vert")
    {
        spvFileName += "_V";
    }
    else if (extension == ".frag")
    {
        spvFileName += "_F";
    }
    else
    {
        return {};
    }
    
    spvFileName.replace_extension(".spv");
    return spvFileName;
}

void Vulkan::ReloadModule(const Filepath& aFilename,vk::Device aDevice,vk::ShaderStageFlagBits aStage)
{
    const Filepath spvDirectory = Wonder::GetBinPath() / "Shaders";
    
    if (localLoadedShaders.contains(aFilename.string()))
    {
        aDevice.destroyShaderModule(localLoadedShaders.at(aFilename.string()).module);
    }
    
    Shader shaderToReload;
    shaderToReload.module = Internal::CreateShaderModule(spvDirectory / Internal::GetSpvFilename(aFilename), aDevice);
    shaderToReload.type = aStage;
    shaderToReload.shaderInfo = MakeShaderInfo(shaderToReload.module, aStage);

    localLoadedShaders.insert_or_assign(aFilename.string(),shaderToReload);
}

void Vulkan::Internal::LoadAllShaderNamesAndPaths()
{
    Filepath shaderFolderPath = Wonder::GetSourcePath() / "Engine/Shaders";
    
    for (auto& item : std::filesystem::recursive_directory_iterator(shaderFolderPath))
    {
        Filepath extension = item.path().extension();
        
        Tga::StringId nameId = Tga::StringRegistry::RegisterOrGetString(item.path().filename().string());
        
        if (extension == ".vert")
        {
            localVertexNamesAndPaths.emplace(nameId,item.path());
            localAllShaderNamesAndPaths.emplace(nameId,item.path());
        }
        else if (extension == ".frag")
        {
            localFragmentNamesAndPaths.emplace(nameId,item.path());
            localAllShaderNamesAndPaths.emplace(nameId,item.path());
        }
    }
    
    localHasLoadedNames = true;
}
