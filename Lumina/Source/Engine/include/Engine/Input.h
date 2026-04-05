#pragma once

#include <cstdint>
#include <windows.h>

enum class EKeyCode : uint16_t
{
    W = 'W', A = 'A', S = 'S', D = 'D',
    Q = 'Q', E = 'E',
    Space = VK_SPACE,
    SHIFT = VK_SHIFT,
    Esc = VK_ESCAPE,
};

enum class EMouseButton : uint8_t
{
    Left = 0, Right = 1, Middle = 2
};

class Input
{
public:
    static bool IsKeyDown(EKeyCode Key);
    static bool IsMouseButtonDown(EMouseButton Button);

    static float GetMouseX();
    static float GetMouseY();

    static float GetMouseDeltaX();
    static float GetMouseDeltaY();

    static void Init();
    static void Update();
    static void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

private:
    static bool s_KeyStates[256];
    static bool s_MouseStates[3];

    static float s_MouseX;
    static float s_MouseY;
    static float s_MouseLastX;
    static float s_MouseLastY;
    static float s_MouseDeltaX;
    static float s_MouseDeltaY;
    static bool s_bFirstMouse;
};