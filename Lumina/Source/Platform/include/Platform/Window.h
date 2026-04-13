#pragma once

#include <functional>

#include "IWindow.h"

#include <memory>
#include <string>
using pfnWndProc_t = LRESULT(CALLBACK*)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct FWindowDesc
{
    int Width = -1;
    int Height = -1;
    HINSTANCE hInstance = nullptr;
    pfnWndProc_t pfnWindowProcedure = nullptr;
    IWindowOwner* pWindowOwner = nullptr;
    bool bFullScreen = false;
    int PreferredDisplay = 0;
    int iShowCommand;
    std::string WindowName;
    std::function<void(HWND, const std::string&)> OnRegisterWindowName;
};

struct WindowClass final
{
public:
    WindowClass(const wchar_t* Name, HINSTANCE hInstance, ::WNDPROC Procedure = ::DefWindowProc);
    ~WindowClass();

    [[nodiscard]] const wchar_t* GetName() const
    {
        return mName;
    }

    WindowClass(const WindowClass&) = delete;
    WindowClass& operator=(const WindowClass&) = delete;

private:
    wchar_t mName[128]{};
};

class Window : public IWindow
{
public:
    Window(const wchar_t* Title, FWindowDesc& InitParameters);

    [[nodiscard]] HWND GetHWND() const;

    void Show() override;
    void Minimize() override;
    void ToggleWindowedFullScreen(FSwapChain* pSwapChain) override;
    void Close() override;
    void SetMouseCapture(bool bCapture) override;

    inline void OnResize(int Width, int Height)
    {
        mWidth = Width;
        mHeight = Height;
    }

    inline void SetFullscreen(bool bIsFullscreen)
    {
        mIsFullscreen = bIsFullscreen;
    }

private:

    [[nodiscard]] bool IsClosedImpl() const override { return mIsClosed; }
    [[nodiscard]] bool IsFullscreenImpl() const override { return mIsFullscreen; }
    [[nodiscard]] bool IsMouseCapturedImpl() const override { return mIsMouseCaptured; }
    [[nodiscard]] int GetWidthImpl() const override { return mWidth; }
    [[nodiscard]] int GetHeightImpl() const override { return mHeight; }
    [[nodiscard]] int GetFullscreenWidthImpl() const override { return FSWidth; }
    [[nodiscard]] int GetFullscreenHeightImpl() const override { return FSHeight; }

private:
    std::unique_ptr<WindowClass> mWindowClass;
    HWND mHwnd = nullptr;
    RECT mRect{};
    bool mIsClosed = false;
    int mWidth = -1;
    int mHeight = -1;
    bool mIsFullscreen = false;
    UINT mWindowStyle{};
    int FSWidth = -1;
    int FSHeight = -1;
    bool mIsMouseCaptured = false;
    bool IsOnHDRCapableDisplay = false;
};