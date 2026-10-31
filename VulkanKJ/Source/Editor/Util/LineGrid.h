#pragma once
#include <cstdint>
#include <glm/vec3.hpp>
#include <Vulkan/VkMesh.h>

namespace Wonder
{
    class LineGrid
    {
    public:
        void MakeLines(glm::vec3 aCameraPosition,int aCellCount);
    
        const std::vector<Vulkan::LineVertex>& GetLinePoints() const;
    private:
        std::vector<Vulkan::LineVertex> myLinePoints;
    };
}

