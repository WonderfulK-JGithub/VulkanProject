#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Wonder
{
    inline void CopyMatrixRotation(glm::mat4& aDest,const glm::mat4& aSrc)
    {
        aDest[0] = aSrc[0];
        aDest[1] = aSrc[1];
        aDest[2] = aSrc[2];
    }

    inline void DecomposeMatrix(const glm::mat4x4& aMatrix,glm::vec3& aTranslation,glm::quat& aRotation,glm::vec3& aScale)
    {
        aScale.x = glm::length(aMatrix[0]);
        aScale.y = glm::length(aMatrix[1]);
        aScale.z = glm::length(aMatrix[2]);
        
        aTranslation = make_vec3(aMatrix[3]);
        
        if (aScale.x == 0.0f || aScale.y == 0.0f || aScale.z == 0.0f)
        {
            return;
        }
        
        aRotation = glm::quat_cast(glm::mat3x3{
            glm::make_vec3(aMatrix[0]) / aScale.x,
            glm::make_vec3(aMatrix[1]) / aScale.y,
            glm::make_vec3(aMatrix[2]) / aScale.z,
        });
    }
}
