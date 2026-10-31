#include "MouseInput.h"

#include <array>

#include "imgui_impl_glfw.h"

namespace MouseInput
{
    static std::array<bool,16> localPendingInput;
    static std::array<bool,16> localCurrentInput;
    static std::array<bool,16> localPreviousInput;
    
    static glm::i64vec2 localPendingPosition;
    static glm::i64vec2 localCurrentPosition;
    static glm::i64vec2 localCurrentDeltaPosition;
    
    static glm::i64vec2 localPendingScroll;
    static glm::i64vec2 localCurrentScroll;
    
    void MouseInputCallback([[maybe_unused]] GLFWwindow* aWindow, const int aButton, const int aAction,[[maybe_unused]] int aMods)
    {
        if (aAction == GLFW_PRESS)
        {
            localPendingInput[aButton] = true;
        }
        else if (aAction == GLFW_RELEASE)
        {
            localPendingInput[aButton] = false;
        }
        
        ImGui_ImplGlfw_MouseButtonCallback(aWindow, aButton, aAction, aMods);
    }

    void CursorPosCallback([[maybe_unused]]GLFWwindow* aWindow, const double aX, const double aY)
    {
        localPendingPosition = glm::i64vec2(aX, aY);
        ImGui_ImplGlfw_CursorPosCallback(aWindow, aX, aY);
    }

    void ScrollCallback([[maybe_unused]]GLFWwindow* aWindow, const double aX, const double aY)
    {
        localPendingScroll = glm::i64vec2(aX, aY);
        ImGui_ImplGlfw_ScrollCallback(aWindow, aX, aY);
    }

    void UpdateMouseInput()
    {
        localPreviousInput = localCurrentInput;
        localCurrentInput = localPendingInput;
        
        localCurrentDeltaPosition = localPendingPosition - localCurrentPosition;
        localCurrentPosition = localPendingPosition;
        
        localCurrentScroll = localPendingScroll;
    }

    bool IsButtonPressed(MouseButton aButton)
    {
        return localCurrentInput[static_cast<int>(aButton)] && !localPreviousInput[static_cast<int>(aButton)];
    }

    bool IsButtonReleased(MouseButton aButton)
    {
        return !localCurrentInput[static_cast<int>(aButton)] && localPreviousInput[static_cast<int>(aButton)];
    }

    bool IsButtonDown(MouseButton aButton)
    {
        return localCurrentInput[static_cast<int>(aButton)];
    }

    float GetDeltaX()
    {
        return static_cast<float>(localCurrentDeltaPosition.x);
    }

    float GetDeltaY()
    {
        return static_cast<float>(localCurrentDeltaPosition.y);
    }

    glm::vec2 GetDelta()
    {
        return {static_cast<float>(localCurrentDeltaPosition.x),static_cast<float>(localCurrentDeltaPosition.y)};
    }

    glm::vec2 GetPosition()
    {
        return {static_cast<float>(localCurrentPosition.x),static_cast<float>(localCurrentPosition.y)};
    }

    float GetScroll()
    {
        return static_cast<float>(localCurrentScroll.y);
    }
}


