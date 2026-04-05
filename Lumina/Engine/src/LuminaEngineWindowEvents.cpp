#include "Engine/LuminaEngine.h"
#include "Logger/Logger.h"
#include "ImGUI/imgui.h"
#include <windowsx.h>

#include "Engine/Input.h"

void LuminaEngine::OnWindowCreate(HWND hWnd)
{
    Log::Info("Window Created");
}

void LuminaEngine::OnWindowResize(HWND hWnd)
{
}

void LuminaEngine::OnWindowMinimize(HWND hwnd)
{
}

void LuminaEngine::OnWindowFocus(HWND hwnd)
{
}

void LuminaEngine::OnWindowLoseFocus(HWND hwnd)
{
}

void LuminaEngine::OnWindowClose(HWND hwnd)
{
    Log::Info("Closing Window");
    Quit();
}

void LuminaEngine::OnToggleFullscreen(HWND hWnd)
{
}

void LuminaEngine::OnWindowActivate(HWND hWnd)
{
}

void LuminaEngine::OnWindowDeactivate(HWND hWnd)
{
}

void LuminaEngine::OnWindowMove(HWND hwnd_, int x, int y)
{
}

void LuminaEngine::OnDisplayChange(HWND hwnd_, int ImageDepthBitsPerPixel, int ScreenWidth, int ScreenHeight)
{
}

void LuminaEngine::OnKeyDown(HWND hwnd, WPARAM wParam)
{
}

void LuminaEngine::OnKeyUp(HWND hwnd, WPARAM wParam)
{
}

void LuminaEngine::OnMouseButtonDown(HWND hwnd, WPARAM wParam, bool bIsDoubleClick)
{
}

void LuminaEngine::OnMouseButtonUp(HWND hwnd, WPARAM wParam)
{
}

void LuminaEngine::OnMouseScroll(HWND hwnd, short scroll)
{
}

void LuminaEngine::OnMouseMove(HWND hwnd, long x, long y)
{
}

void LuminaEngine::OnMouseInput(HWND hwnd, LPARAM lParam)
{
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }

    auto* pWindow = reinterpret_cast<IWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!pWindow)
    {
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    Input::ProcessMessage(message, wParam, lParam);

    switch (message)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            pWindow->pOwner->OnWindowMinimize(hWnd);
        }
        else
        {
            pWindow->pOwner->OnWindowResize(hWnd);
        }
        return 0;

    case WM_SETFOCUS:
        pWindow->pOwner->OnWindowFocus(hWnd);
        return 0;

    case WM_KILLFOCUS:
        pWindow->pOwner->OnWindowLoseFocus(hWnd);
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        pWindow->pOwner->OnKeyDown(hWnd, wParam);
        return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        pWindow->pOwner->OnKeyUp(hWnd, wParam);
        return 0;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        pWindow->pOwner->OnMouseButtonDown(hWnd, wParam, false);
        return 0;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        pWindow->pOwner->OnMouseButtonUp(hWnd, wParam);
        return 0;

    case WM_MOUSEWHEEL:
        pWindow->pOwner->OnMouseScroll(hWnd, GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;

    case WM_MOUSEMOVE:
        pWindow->pOwner->OnMouseMove(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_CLOSE:
        if (pWindow->pOwner)
        {
            pWindow->Close();
            return 0;
        }

    case WM_DESTROY:
        pWindow->pOwner->OnWindowClose(hWnd);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);;
    }

    // return DefWindowProcW(hWnd, message, wParam, lParam);
}