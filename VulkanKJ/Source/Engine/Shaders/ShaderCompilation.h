#pragma once
#include <string>
#include <shaderc/shaderc.h>

#include "Rendering/RenderStage.h"
#include "Util/FileManagement/FileTypes.h"

namespace Vulkan
{
    bool CompileShader(const Filepath& aInputFile,
    const Filepath& aOutputFile,
    shaderc_shader_kind aShaderKind);
    
    shaderc_shader_kind GetShaderKind(const Filepath& aFilename);
    vk::ShaderStageFlagBits GetShaderStage(const Filepath& aExtension);
}
