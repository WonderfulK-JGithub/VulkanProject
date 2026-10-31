#pragma once
#include <vulkan/vulkan.hpp>

#include "Vulkan/RenderStructs.h"

namespace Vulkan
{
    void CreatePipelineLayouts();
    void DestroyPipelineLayouts();
    
    vk::PipelineLayout GetPipelineLayout(RenderStageType aRenderStageType);
}
