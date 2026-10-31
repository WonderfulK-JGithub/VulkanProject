#include "CameraController.h"

#include <glm/gtx/euler_angles.hpp>

#include "Engine.h"
#include "KeyboardInput.h"
#include "MouseInput.h"
#include "Math/MatrixUtil.h"

void CameraController::SetPosition(const glm::vec3& position)
{
    myPosition = position;
}

void CameraController::SetProjection(const float aWidth,const float aHeight)
{
    myProjection = glm::perspective(
            glm::radians(45.f),
            static_cast<float>(aWidth) / static_cast<float>(aHeight),
            0.001f,
            100.f);
    myProjection[1][1] *= -1.f;
}

void CameraController::Update()
{
    float deltaTime = Engine::GetInstance().GetDeltaTime();
    if (MouseInput::IsButtonDown(MouseInput::MouseButton::Right))
    {
        myRotation.x -= MouseInput::GetDeltaY() * 0.01f;
        myRotation.y -= MouseInput::GetDeltaX() * 0.01f;
        
        float right = (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::D) ? 1.0f : 0.0f) - 
            (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::A) ? 1.0f : 0.0f);
        
        float up = (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::E) ? 1.0f : 0.0f) - 
            (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::Q) ? 1.0f : 0.0f);
        
        float forward = (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::W) ? 1.0f : 0.0f) - 
            (KeyboardInput::IsKeyDown(KeyboardInput::Keycode::S) ? 1.0f : 0.0f);
        
        float speed = deltaTime * 3;
        
        Wonder::CopyMatrixRotation(myTransform,glm::yawPitchRoll(myRotation.y,myRotation.x,myRotation.z));
        
        myPosition += right * myTransform[0] * speed;
        myPosition += up * myTransform[1] * speed;
        myPosition += -forward * myTransform[2] * speed;
    }
    
    myTransform[3] = glm::vec4(glm::lerp(xyz(myTransform[3]),myPosition,deltaTime * 5.f),1.f);
}

void CameraController::SetSceneCameraInfo(SceneRenderer::SceneObjectInfo& aSceneObjectInfo) const
{
    const glm::mat4 viewMatrix = GetView();
    
    aSceneObjectInfo.cameraMatrixData.view = viewMatrix;
    aSceneObjectInfo.cameraMatrixData.projection = myProjection;
    aSceneObjectInfo.cameraMatrixData.viewProjection = myProjection * viewMatrix;

    aSceneObjectInfo.cameraVectorData.right = myTransform[0];
    aSceneObjectInfo.cameraVectorData.up = myTransform[1];
    aSceneObjectInfo.cameraVectorData.forward = -myTransform[2];
}

glm::mat4x4 CameraController::GetView() const
{
    return glm::inverse(myTransform);
}

const glm::mat4x4& CameraController::GetProjection() const
{
    return myProjection;
}

const glm::mat4x4& CameraController::GetTransform() const
{
    return myTransform;
}
