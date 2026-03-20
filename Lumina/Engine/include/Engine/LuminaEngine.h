#pragma once

#include "LuminaApp.h"
#include "FrameTimer/FrameTimer.h"
#include "Settings.h"
#include "Platform/IWindow.h"
#include "Platform/Window.h"

#include <string>
#include <windows.h>
#include <memory>
#include <unordered_map>

using WindowNameLookup_t = std::unordered_map<HWND, std::string>;
extern LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

class LuminaEngine : public IWindowOwner
{
public:
    LuminaEngine() = default;

    bool Initialize(FStartupParameters& StartupParameters, LuminaApp* Application);
    void Run();
    void Quit();

    // OS Window Events
    void OnWindowCreate(HWND hWnd) override;
    void OnWindowResize(HWND hWnd) override;
    void OnWindowMinimize(HWND hwnd) override;
    void OnWindowFocus(HWND hwnd) override;
    void OnWindowLoseFocus(HWND hwnd) override;
    void OnWindowClose(HWND hwnd) override;
    void OnToggleFullscreen(HWND hWnd) override;
    void OnWindowActivate(HWND hWnd) override;
    void OnWindowDeactivate(HWND hWnd) override;
    void OnWindowMove(HWND hwnd_, int x, int y) override;
    void OnDisplayChange(HWND hwnd_, int ImageDepthBitsPerPixel, int ScreenWidth, int ScreenHeight) override;

    // Keyboard & Mouse Events
    void OnKeyDown(HWND hwnd, WPARAM wParam) override;
    void OnKeyUp(HWND hwnd, WPARAM wParam) override;
    void OnMouseButtonDown(HWND hwnd, WPARAM wParam, bool bIsDoubleClick) override;
    void OnMouseButtonUp(HWND hwnd, WPARAM wParam) override;
    void OnMouseScroll(HWND hwnd, short scroll) override;
    void OnMouseMove(HWND hwnd, long x, long y) override;
    void OnMouseInput(HWND hwnd, LPARAM lParam) override;

    void SetWindowName(HWND hwnd, const std::string& WindowName)
    {
        mWinNameLookup[hwnd] = WindowName;
    }
    void SetWindowName(const std::unique_ptr<Window>& pWindow, const std::string& Name)
    {
        SetWindowName(pWindow->GetHWND(), Name);
    }

private:
    void InitializeWindows(FStartupParameters& StartupParameters);

    std::unique_ptr<Window> mpMainWindow;
    WindowNameLookup_t    mWinNameLookup;

    GraphicsDevice mGraphicsDevice;
    LuminaApp* mCurrentApp{};
    FrameTimer mTimer;
    bool mbIsRunning = false;
};

