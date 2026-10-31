#include "KeyboardInput.h"

#include <array>

#include "imgui_impl_glfw.h"

namespace KeyboardInput
{
    static std::array<bool,GLFW_KEY_LAST + 1> pendingInput;
    static std::array<bool,GLFW_KEY_LAST + 1> currentInput;
    static std::array<bool,GLFW_KEY_LAST + 1> previousInput;
    
    void KeyCallback([[maybe_unused]] GLFWwindow* aWindow, int aKey, [[maybe_unused]] int aScancode, int aAction, [[maybe_unused]] int aMods)
    {
        if (aAction == GLFW_PRESS)
        {
            pendingInput[aKey] = true;
        }
        else if (aAction == GLFW_RELEASE)
        {
            pendingInput[aKey] = false;
        }
        
        ImGui_ImplGlfw_KeyCallback(aWindow, aKey, aScancode, aAction, aMods);
    }

    void CharacterCallback(GLFWwindow* window, unsigned int codepoint)
    {
        ImGui_ImplGlfw_CharCallback(window, codepoint);
    }

    void UpdateInput()
    {
        previousInput = currentInput;
        currentInput = pendingInput;
    }

    bool IsKeyPressed(Keycode aKey)
    {
        return currentInput[static_cast<int>(aKey)] && !previousInput[static_cast<int>(aKey)];
    }

    bool IsKeyReleased(Keycode aKey)
    {
        return !currentInput[static_cast<int>(aKey)] && previousInput[static_cast<int>(aKey)];
    }

    bool IsKeyDown(Keycode aKey)
    {
        return currentInput[static_cast<int>(aKey)];
    }
}


