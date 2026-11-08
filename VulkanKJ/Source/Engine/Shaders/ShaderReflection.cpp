#include "ShaderReflection.h"

#include <fstream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <magic_enum/magic_enum.hpp>

#include "Util/Logging/Logging.h"

namespace ShaderReflection
{
    static std::unordered_map<std::string, PushConstantInfo> localPushConstantDefaultValues;
    static std::unordered_map<std::string, Vulkan::VertexLayoutType> localVertexLayoutTypes;
    static std::unordered_map<std::string, Vulkan::PipelineLayoutType> localPipelineLayoutTypes;
    static std::unordered_map<std::string, VertexToPixelType> localVertexToPixelTypes;
    
    static std::unordered_map<Filepath, ReflectionInfo> localReflectionInfos;
    
    static ReflectionInfo localEmptyReflectionInfo;
}

void ShaderReflection::InitializeDefaultValues()
{
    localPushConstantDefaultValues = {
        {"TextureIndex",{.type = PushConstantType::TextureIndex,.data = std::array<char,16>{}}},
        {"CubemapIndex",{.type = PushConstantType::CubemapIndex,.data = std::array<char,16>{}}},
        {"ColorValue",{.type = PushConstantType::Color,.data = std::array<char,16>{}}},
        {"float",{.type = PushConstantType::Float,.data = std::array<char,16>{}}},
        {"vec2",{.type = PushConstantType::Vector2,.data = std::array<char,16>{}}},
        {"vec3",{.type = PushConstantType::Vector3,.data = std::array<char,16>{}}}
    };
    
    localVertexLayoutTypes = {
        {"\"VertexInputs/MeshVertex.glsl\"",Vulkan::VertexLayoutType::PosColor},
        {"\"VertexInputs/LineVertex.glsl\"",Vulkan::VertexLayoutType::Line},
    };
    
    localPipelineLayoutTypes = {
        {"\"PipelineLayouts/DefaultPipelineLayout.glsl\"",Vulkan::PipelineLayoutType::DefaultPipeline},
    };
    
    localVertexToPixelTypes = {
        {"\"VertexOutputs/ColorOnlyVertexOut.glsl\"",VertexToPixelType::ColorOnly},
        {"\"FragmentInputs/ColorOnlyFragmentIn.glsl\"",VertexToPixelType::ColorOnly},
        {"\"VertexOutputs/FullscreenVertexOut.glsl\"",VertexToPixelType::Fullscreen},
        {"\"FragmentInputs/FullscreenFragmentIn.glsl\"",VertexToPixelType::Fullscreen},
        {"\"VertexOutputs/CubemapVertexOut.glsl\"",VertexToPixelType::Cubemap},
        {"\"FragmentInputs/CubemapFragmentIn.glsl\"",VertexToPixelType::Cubemap},
        {"\"VertexOutputs/RegularVertexOut.glsl\"",VertexToPixelType::Regular},
        {"\"FragmentInputs/RegularFragmentIn.glsl\"",VertexToPixelType::Regular}
    };
}

const ShaderReflection::ReflectionInfo& ShaderReflection::GetShaderReflectionInfo(const Filepath& aShaderPath,bool aForceReset)
{
    using namespace ShaderReflection::Internal;
    
    if (aForceReset == false && localReflectionInfos.contains(aShaderPath))
    {
        return localReflectionInfos.at(aShaderPath);
    }
    
    ReflectionInfo& reflectionInfo = localReflectionInfos[aShaderPath];
    
    LOG_INFO("Reflecting Shader %s", aShaderPath.filename().c_str());
    
    std::ifstream file(aShaderPath);
    if (!file.is_open())
        return localEmptyReflectionInfo;

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string text = buffer.str();

    Goose::StringUtils::RemoveComments(text);
    
    AssignDependenciesTypes(text,reflectionInfo);
    
    reflectionInfo.pushConstantsInfo = GetPushConstantsInfo(text);
    
    if (aShaderPath.extension() == ".vert")
    {
        LOG_INFO("Shader Vertex input: %s",std::string{magic_enum::enum_name(reflectionInfo.vertexLayoutType)}.c_str());
    }
    
    LOG_INFO("Shader VertexToPixel type: %s",std::string{magic_enum::enum_name(reflectionInfo.vertexToPixelType)}.c_str());
    LOG_INFO("Shader Pipeline type: %s",std::string{magic_enum::enum_name(reflectionInfo.pipelineLayoutType)}.c_str());
    LOG_INFO("Shader push constants count: %d",static_cast<int>(reflectionInfo.pushConstantsInfo.size()));
    
    return reflectionInfo;
}

uint32_t ShaderReflection::GetShaderSizeOfType(const PushConstantType aType)
{
    switch (aType)
    {
        case PushConstantType::TextureIndex:
        case PushConstantType::RenderImageIndex:
            return sizeof(uint32_t);
        case PushConstantType::Float:
            return sizeof(float);
        case PushConstantType::Vector2:
            return sizeof(glm::vec2);
        case PushConstantType::Vector3:
            return sizeof(glm::vec3);
        case PushConstantType::Color:
            return sizeof(glm::vec4);
        case PushConstantType::Padding:
            break;
    }
    
    return 0;
}

Goose::MappedVector<std::string, ShaderReflection::PushConstantInfo> ShaderReflection::Internal::GetPushConstantsInfo(
std::string aShaderText)
{
    aShaderText = Goose::StringUtils::GetAfter(aShaderText,"push_constant");
    aShaderText = Goose::StringUtils::GetAfter(aShaderText,"{");
    aShaderText = Goose::StringUtils::GetBefore(aShaderText,"}");
    Goose::StringUtils::RemoveAll(aShaderText,';');
    
    std::vector<std::string> shaderWords = Goose::StringUtils::SplitString(aShaderText," ");
    shaderWords = Goose::StringUtils::SplitStrings(shaderWords,"\n");

    for (auto& word : shaderWords)
    {
        Goose::StringUtils::RemoveAll(word,'\t');
    }
    
    Goose::MappedVector<std::string,ShaderReflection::PushConstantInfo> pushConstants;
    
    uint32_t wordIndex = 0;
    uint32_t paddingCheck = 0;
    
    constexpr uint32_t paddingLength = 16;
    
    while (wordIndex < shaderWords.size())
    {
        if (localPushConstantDefaultValues.contains(shaderWords[wordIndex]))
        {
            PushConstantInfo info = localPushConstantDefaultValues.at(shaderWords[wordIndex]);
            
            uint32_t typeSize =  GetShaderSizeOfType(info.type);
            if (paddingCheck + typeSize > paddingLength)
            {
                PushConstantInfo paddingInfo{};
                paddingInfo.type = PushConstantType::Padding;
                const uint32_t padding = paddingLength - paddingCheck;
                std::copy_n(reinterpret_cast<const char*>(&padding),sizeof(padding),paddingInfo.data.data());
                pushConstants.emplace("[PADDING]",paddingInfo);
                
                paddingCheck = 0;
            }
            
            pushConstants.emplace(shaderWords[wordIndex + 1],localPushConstantDefaultValues.at(shaderWords[wordIndex]));
            wordIndex++;
            
            paddingCheck += typeSize;
            paddingCheck %= paddingLength;
        }
        
        wordIndex++;
    }
    
    return pushConstants;
}

void ShaderReflection::Internal::AssignDependenciesTypes(const std::string& aShaderText, ReflectionInfo& aReflectionInfo)
{
    std::string dependenciesText = Goose::StringUtils::GetBetween(aShaderText,"#pragma region Dependencies","#pragma endregion");
    
    if (dependenciesText.empty())
    {
        LOG_WARNING("Shader is missing [pragma region Dependencies] or [pragma endregion], reflection is takes more time now!");
        dependenciesText = aShaderText;
    }
    
    std::vector<std::string> shaderWords = Goose::StringUtils::SplitString(dependenciesText," ");
    shaderWords = Goose::StringUtils::SplitStrings(shaderWords,"\n");
    
    uint32_t wordIndex = 0;
    while (wordIndex < shaderWords.size())
    {
        if (shaderWords[wordIndex] != "#include")
        {
            wordIndex++;
            continue;
        }
        
        wordIndex++;
        
        auto pipelineIt = localPipelineLayoutTypes.find(shaderWords[wordIndex]);
        if (pipelineIt != localPipelineLayoutTypes.end())
        {
            aReflectionInfo.pipelineLayoutType = pipelineIt->second;
        }
        
        auto vertexToPixelIt = localVertexToPixelTypes.find(shaderWords[wordIndex]);
        if (vertexToPixelIt != localVertexToPixelTypes.end())
        {
            aReflectionInfo.vertexToPixelType = vertexToPixelIt->second;
        }
        
        auto vertexTypeIt = localVertexLayoutTypes.find(shaderWords[wordIndex]);
        if (vertexTypeIt != localVertexLayoutTypes.end())
        {
            aReflectionInfo.vertexLayoutType = vertexTypeIt->second;
        }
    }
}
