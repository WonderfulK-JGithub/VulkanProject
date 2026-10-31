#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace MouseInput
{
    enum class MouseButton
    {
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Right = GLFW_MOUSE_BUTTON_RIGHT,
        Middle = GLFW_MOUSE_BUTTON_MIDDLE,
    };
    
    void MouseInputCallback(GLFWwindow* aWindow, int aButton, int aAction, int aMods);
    void CursorPosCallback(GLFWwindow* aWindow, double aX, double aY);
    void ScrollCallback(GLFWwindow* aWindow, double aX, double aY);
    void UpdateMouseInput();
    
    bool IsButtonPressed(MouseButton aButton);
    bool IsButtonReleased(MouseButton aButton);
    bool IsButtonDown(MouseButton aButton);
    
    float GetDeltaX();
    float GetDeltaY();
    glm::vec2 GetDelta();
    
    glm::vec2 GetPosition();
    float GetScroll();
}
