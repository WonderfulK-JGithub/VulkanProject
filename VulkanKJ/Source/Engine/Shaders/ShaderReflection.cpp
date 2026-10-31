#include "ShaderReflection.h"

#include <fstream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Util/Logging/Logging.h"

namespace ShaderReflection
{
    static std::unordered_map<std::string, PushConstantInfo> localPushConstantDefaultValues;
}

void ShaderReflection::InitializeDefaultValues()
{
    localPushConstantDefaultValues = {
        {"TextureIndex",{.type = PushConstantType::TextureIndex,.data = std::array<char,16>{}}},
        {"ColorValue",{.type = PushConstantType::Color,.data = std::array<char,16>{}}},
        {"float",{.type = PushConstantType::Float,.data = std::array<char,16>{}}},
        {"vec2",{.type = PushConstantType::Vector2,.data = std::array<char,16>{}}},
        {"vec3",{.type = PushConstantType::Vector3,.data = std::array<char,16>{}}}
    };
}

Goose::MappedVector<std::string,ShaderReflection::PushConstantInfo> ShaderReflection::GetShaderPushConstantTypes(const Filepath& aShaderPath)
{
    LOG_INFO("Reflecting Shader %s", aShaderPath.filename().c_str());
    
    std::ifstream file(aShaderPath);
    if (!file.is_open())
        return {};

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string text = buffer.str();
    text = Goose::StringUtils::GetAfter(text,"push_constant");
    text = Goose::StringUtils::GetAfter(text,"{");
    text = Goose::StringUtils::GetBefore(text,"}");
    Goose::StringUtils::RemoveAll(text,';');
    
    std::vector<std::string> shaderWords = Goose::StringUtils::SplitString(text," ");
    shaderWords = Goose::StringUtils::SplitString(shaderWords,"\n");

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
        
        LOG_INFO("Word: %s", shaderWords[wordIndex].c_str());
        wordIndex++;
    }
    
    return pushConstants;
}

uint32_t ShaderReflection::GetShaderSizeOfType(PushConstantType aType)
{
    switch (aType)
    {
        case PushConstantType::TextureIndex:
            return sizeof(uint32_t);
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
