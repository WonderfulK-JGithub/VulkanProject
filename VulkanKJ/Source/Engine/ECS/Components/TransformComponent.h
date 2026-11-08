#pragma once
#include <glm/mat4x4.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>

#include "Math/MatrixUtil.h"

namespace Goose
{
    struct TransformTRS
    {
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale{1.f,1.f,1.f};
    };
    
    struct TransformComponent
    {
        TransformComponent() = default;
        TransformComponent(const glm::mat4x4& aMatrix)
        {
            Wonder::DecomposeMatrix(aMatrix,position,rotation,scale);
            rotation = glm::normalize(rotation);
        }
        
        TransformComponent(const TransformTRS& aTRS)
        {
            position = aTRS.translation;
            rotation = glm::quat(aTRS.rotation);
            scale = aTRS.scale;
        }
        
        TransformComponent(const glm::vec3& aPosition, const glm::quat& aRotation, const glm::vec3& aScale) 
            : position(aPosition), rotation(aRotation), scale(aScale) {}
        
        operator glm::mat4x4() const
        {
            glm::mat4x4 result = glm::scale(glm::toMat4(rotation), scale);
            result[3] = glm::vec4(position, 1.0f);
            return result;
        }
        
        TransformComponent& operator*=(const glm::mat4x4& aOther)
        {
            glm::mat4x4 result = *this;
            result *= aOther;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(result, position, rotation, scale, skew, perspective);
            return *this;
        }
        
        bool operator==(const TransformComponent& aOther) const
        {
            return position == aOther.position && rotation == aOther.rotation && scale == aOther.scale;
        }
        
        glm::vec3 position{};
        glm::quat rotation{ };
        glm::vec3 scale{1.f,1.f,1.f};
    };
}
