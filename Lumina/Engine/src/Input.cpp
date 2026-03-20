#include "Engine/Input.h"

#include <windowsx.h>

bool Input::s_KeyStates[256] = { false };
bool Input::s_MouseStates[3] = { false };
float Input::s_MouseX = 0.0f; float Input::s_MouseY = 0.0f;
float Input::s_MouseLastX = 0.0f; float Input::s_MouseLastY = 0.0f;
float Input::s_MouseDeltaX = 0.0f; float Input::s_MouseDeltaY = 0.0f;
bool Input::s_bFirstMouse = true;

void Input::Init() {}

void Input::Update()
{
    s_MouseDeltaX = 0.0f;
    s_MouseDeltaY = 0.0f;
}

void Input::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wParam < 256) s_KeyStates[wParam] = true;
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam < 256) s_KeyStates[wParam] = false;
        break;

    // 鼠标按键
    case WM_LBUTTONDOWN: s_MouseStates[(int)EMouseButton::Left] = true; break;
    case WM_LBUTTONUP:   s_MouseStates[(int)EMouseButton::Left] = false; break;
    case WM_RBUTTONDOWN: s_MouseStates[(int)EMouseButton::Right] = true; break;
    case WM_RBUTTONUP:   s_MouseStates[(int)EMouseButton::Right] = false; break;
    case WM_MBUTTONDOWN: s_MouseStates[(int)EMouseButton::Middle] = true; break;
    case WM_MBUTTONUP:   s_MouseStates[(int)EMouseButton::Middle] = false; break;

    // 鼠标移动
    case WM_MOUSEMOVE:
    {
        float x = static_cast<float>(GET_X_LPARAM(lParam));
        float y = static_cast<float>(GET_Y_LPARAM(lParam));

        if (s_bFirstMouse)
        {
            s_MouseLastX = x;
            s_MouseLastY = y;
            s_bFirstMouse = false;
        }

        s_MouseX = x;
        s_MouseY = y;

        s_MouseDeltaX += (s_MouseX - s_MouseLastX);
        s_MouseDeltaY += (s_MouseY - s_MouseLastY);

        s_MouseLastX = s_MouseX;
        s_MouseLastY = s_MouseY;
        break;
    }
    default: break;
    }
}

bool Input::IsKeyDown(EKeyCode Key) { return s_KeyStates[static_cast<uint16_t>(Key)]; }
bool Input::IsMouseButtonDown(EMouseButton Button) { return s_MouseStates[static_cast<uint8_t>(Button)]; }
float Input::GetMouseX() { return s_MouseX; }
float Input::GetMouseY() { return s_MouseY; }
float Input::GetMouseDeltaX() { return s_MouseDeltaX; }
float Input::GetMouseDeltaY() { return s_MouseDeltaY; }