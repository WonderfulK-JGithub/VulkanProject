#pragma once
#include <glm/mat4x4.hpp>

namespace Wonder
{
    void CopyMatrixRotation(glm::mat4& aDest,const glm::mat4& aSrc)
    {
        aDest[0] = aSrc[0];
        aDest[1] = aSrc[1];
        aDest[2] = aSrc[2];
    }
}
