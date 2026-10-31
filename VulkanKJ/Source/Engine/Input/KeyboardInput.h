#pragma once
#include <GLFW/glfw3.h>

namespace KeyboardInput
{
    enum class Keycode : int
    {
        A = GLFW_KEY_A,
        B = GLFW_KEY_B,
        C = GLFW_KEY_C,
        D = GLFW_KEY_D,
        E = GLFW_KEY_E,
        F = GLFW_KEY_F,
        G = GLFW_KEY_G,
        H = GLFW_KEY_H,
        I = GLFW_KEY_I,
        J = GLFW_KEY_J,
        K = GLFW_KEY_K,
        L = GLFW_KEY_L,
        M = GLFW_KEY_M,
        N = GLFW_KEY_N,
        O = GLFW_KEY_O,
        P = GLFW_KEY_P,
        Q = GLFW_KEY_Q,
        R = GLFW_KEY_R,
        S = GLFW_KEY_S,
        T = GLFW_KEY_T,
        U = GLFW_KEY_U,
        V = GLFW_KEY_V,
        W = GLFW_KEY_W,
        X = GLFW_KEY_X,
        Y = GLFW_KEY_Y,
        Z = GLFW_KEY_Z,
        KEY_0 = GLFW_KEY_0,
        KEY_1 = GLFW_KEY_1,
        KEY_2 = GLFW_KEY_2,
        KEY_3 = GLFW_KEY_3,
        KEY_4 = GLFW_KEY_4,
        KEY_5 = GLFW_KEY_5,
        KEY_6 = GLFW_KEY_6,
        KEY_7 = GLFW_KEY_7,
        KEY_8 = GLFW_KEY_8,
        KEY_9 = GLFW_KEY_9,
        UP = GLFW_KEY_UP,
        DOWN = GLFW_KEY_DOWN,
        LEFT = GLFW_KEY_LEFT,
        RIGHT = GLFW_KEY_RIGHT,
        Space = GLFW_KEY_SPACE,
        Enter = GLFW_KEY_ENTER,
        Backspace = GLFW_KEY_BACKSPACE,
        Tab = GLFW_KEY_TAB,
        Escape = GLFW_KEY_ESCAPE,
        Shift = GLFW_KEY_LEFT_SHIFT,
        Control = GLFW_KEY_LEFT_CONTROL,
        Alt = GLFW_KEY_LEFT_ALT,
        MouseLeft = GLFW_MOUSE_BUTTON_LEFT,
        MouseRight = GLFW_MOUSE_BUTTON_RIGHT,
        MouseMiddle = GLFW_MOUSE_BUTTON_MIDDLE,
    };
    
    void KeyCallback(GLFWwindow* aWindow, int aKey, int aScancode, int aAction, int aMods);
    void CharacterCallback(GLFWwindow* window, unsigned int codepoint);

    void UpdateInput();
    
    bool IsKeyPressed(Keycode aKey);
    bool IsKeyReleased(Keycode aKey);
    bool IsKeyDown(Keycode aKey);
}
