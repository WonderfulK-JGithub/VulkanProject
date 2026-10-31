#pragma once
#include <glm/mat4x4.hpp>

namespace SceneRenderer
{
    struct SceneObjectInfo;
}

class CameraController
{
public:
    void SetPosition(const glm::vec3& position);
    void SetProjection(float aWidth, float aHeight);
    void Update();
    
    void SetSceneCameraInfo(SceneRenderer::SceneObjectInfo& aSceneObjectInfo) const;
    
    glm::mat4x4 GetView() const;
    const glm::mat4x4& GetProjection() const;
    const glm::mat4x4& GetTransform() const;
    
private:
    glm::mat4x4 myProjection;
    glm::mat4x4 myTransform = glm::mat4x4(1.f);
    
    glm::vec3 myPosition;
    glm::vec3 myRotation;
};
