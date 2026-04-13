// Interfaces for windows
// IWindowOwner is for engine to deal with events

#pragma once

#include "../../../Renderer/include/Renderer/D3D12Core/Core/FSwapChain.h"

#include <windows.h>

class IWindowOwner
{
public:
    virtual ~IWindowOwner() = default;

    virtual void OnWindowCreate(HWND hwnd_) = 0;
    virtual void OnWindowResize(HWND) = 0;
    virtual void OnToggleFullscreen(HWND) = 0;
    virtual void OnWindowMinimize(HWND hwnd_) = 0;
    virtual void OnWindowFocus(HWND hwnd_) = 0;
    virtual void OnWindowLoseFocus(HWND hwnd_) = 0;
    virtual void OnWindowClose(HWND hwnd_) = 0;
    virtual void OnWindowActivate(HWND hwnd_) = 0;
    virtual void OnWindowDeactivate(HWND hwnd_) = 0;
    virtual void OnWindowMove(HWND hwnd_, int x, int y) = 0;
    virtual void OnDisplayChange(HWND hwnd_, int ImageDepthBitsPerPixel, int ScreenWidth, int ScreenHeight) = 0;

    virtual void OnKeyDown(HWND, WPARAM) = 0;
    virtual void OnKeyUp(HWND, WPARAM) = 0;

    virtual void OnMouseButtonDown(HWND hwnd, WPARAM wParam, bool bIsDoubleClick) = 0;
    virtual void OnMouseButtonUp(HWND, WPARAM) = 0;
    virtual void OnMouseScroll(HWND hwnd, short scrollDirection) = 0;
    virtual void OnMouseMove(HWND hwnd, long x, long y) = 0;
    virtual void OnMouseInput(HWND hwnd, LPARAM lParam) = 0;
};

struct IWindow
{
    IWindow(IWindowOwner* pWindowOwner) : pOwner(pWindowOwner) {}
    IWindow() = default;
    IWindow(const IWindow&) = delete;
    IWindow& operator=(const IWindow&) = delete;
    virtual ~IWindow() = default;

    virtual void Show() = 0;
    virtual void ToggleWindowedFullScreen(FSwapChain* pSwapChain) = 0;
    virtual void Minimize() = 0;
    virtual void SetMouseCapture(bool bCapture) = 0;
    virtual void Close() = 0;

    [[nodiscard]] bool IsClosed() const;
    [[nodiscard]] bool IsFullscreen() const;
    [[nodiscard]] bool IsMouseCaptured() const;

    [[nodiscard]] int GetWidth() const { return GetWidthImpl(); }
    [[nodiscard]] int GetHeight() const { return GetHeightImpl(); }
    [[nodiscard]] int GetFullscreenWidth() const { return GetFullscreenWidthImpl(); }
    [[nodiscard]] int GetFullscreenHeight() const { return GetFullscreenHeightImpl(); }

    IWindowOwner* pOwner = nullptr;
private:
    [[nodiscard]] virtual bool IsClosedImpl() const = 0;
    [[nodiscard]] virtual bool IsFullscreenImpl() const = 0;
    [[nodiscard]] virtual bool IsMouseCapturedImpl() const = 0;
    [[nodiscard]] virtual int GetWidthImpl() const = 0;
    [[nodiscard]] virtual int GetHeightImpl() const = 0;
    [[nodiscard]] virtual int GetFullscreenWidthImpl() const = 0;
    [[nodiscard]] virtual int GetFullscreenHeightImpl() const = 0;
};