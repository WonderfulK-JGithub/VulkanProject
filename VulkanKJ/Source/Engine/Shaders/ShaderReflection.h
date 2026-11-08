#pragma once
#include <array>
#include <vector>

#include "Util/Container/MappedVector.hpp"
#include "Util/FileManagement/FileTypes.h"
#include "Vulkan/VkMesh.h"
#include "Vulkan/VkInit/PipelineLayout.h"

namespace ShaderReflection
{
    enum class VertexToPixelType
    {
        Invalid,
        Regular,
        ColorOnly,
        Fullscreen,
        Cubemap,
    };
    
    enum class PushConstantType
    {
        TextureIndex,
        RenderImageIndex,
        CubemapIndex,
        Float,
        Vector2,
        Vector3,
        Color,
        Padding,
    };
    
    struct PushConstantInfo
    {
        PushConstantType type{};
        std::array<char,16> data;
    };
    
    struct ReflectionInfo
    {
        Goose::MappedVector<std::string,ShaderReflection::PushConstantInfo> pushConstantsInfo;
        Vulkan::VertexLayoutType vertexLayoutType = Vulkan::VertexLayoutType::Empty;
        VertexToPixelType vertexToPixelType = VertexToPixelType::Invalid;
        Vulkan::PipelineLayoutType pipelineLayoutType = Vulkan::PipelineLayoutType::Invalid;
    };
    
    void InitializeDefaultValues();
    const ReflectionInfo& GetShaderReflectionInfo(const Filepath& aShaderPath,bool aForceReset = false);
    
    uint32_t GetShaderSizeOfType(PushConstantType aType);

    namespace Internal
    {
        Goose::MappedVector<std::string,ShaderReflection::PushConstantInfo> GetPushConstantsInfo(std::string aShaderText);
        
        void AssignDependenciesTypes(const std::string& aShaderText,ReflectionInfo& aReflectionInfo);
    }
}
