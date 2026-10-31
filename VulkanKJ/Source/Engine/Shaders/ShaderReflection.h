#pragma once
#include <array>
#include <vector>

#include "Util/Container/MappedVector.hpp"
#include "Util/FileManagement/FileTypes.h"

namespace ShaderReflection
{
    enum class PushConstantType
    {
        TextureIndex,
        RenderImageIndex,
        Float,
        Vector2,
        Vector3,
        Color,
        Padding,
    };
    
    struct PushConstantInfo
    {
        PushConstantType type;
        std::array<char,16> data;
    };
    
    void InitializeDefaultValues();
    Goose::MappedVector<std::string,PushConstantInfo> GetShaderPushConstantTypes(const Filepath& aShaderPath);
    
    uint32_t GetShaderSizeOfType(PushConstantType aType);
}
