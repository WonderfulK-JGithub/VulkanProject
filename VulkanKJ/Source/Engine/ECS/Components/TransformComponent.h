#pragma once
#include <glm/mat4x4.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>

namespace Goose
{
    struct TransformComponent
    {
        TransformComponent() = default;
        TransformComponent(const glm::mat4x4& aMatrix)
        {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(aMatrix, position, rotation, scale, skew, perspective);
            rotation = glm::normalize(rotation);
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
