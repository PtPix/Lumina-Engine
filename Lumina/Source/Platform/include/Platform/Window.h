#pragma once

#include <functional>
#include <windows.h>
#include <memory>
#include <string>

struct FWin32Message
{
    HWND Hwnd = nullptr;
    UINT Message = 0;
    WPARAM WParam = 0;
    LPARAM LParam = 0;
};

struct FWindowCallbacks
{
    std::function<bool(const FWin32Message&, LRESULT&)> fnNativeMessageHandler;

    std::function<void(class FWindow&)> fnOnCreate;
    std::function<void(class FWindow&)> fnOnDestroy;
    std::function<void(class FWindow&, uint32_t Width, uint32_t Height)> fnOnResize;
    std::function<void(class FWindow&)> fnOnMinimize;
    std::function<void(class FWindow&)> fnOnFocus;
    std::function<void(class FWindow&)> fnOnLoseFocus;
};

struct FWindowDesc
{
    int Width = 1280;
    int Height = 720;
    HINSTANCE hInstance = nullptr;
    bool bFullScreen = false;
    int PreferredDisplay = 0;
    int iShowCommand = SW_SHOWDEFAULT;
    std::string WindowName;
    std::wstring WindowTitle = L"Lumina";
    FWindowCallbacks Callbacks;
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

class FWindow
{
public:
    FWindow(const wchar_t* Title, const FWindowDesc& InitParameters);

    [[nodiscard]] HWND GetHWND() const;

    void Show();
    void Minimize();
    void ToggleWindowedFullScreen();
    void Close();
    void SetMouseCapture(bool bCapture);

    inline void OnResize(int Width, int Height)
    {
        mWidth = Width;
        mHeight = Height;
    }

    inline void SetFullscreen(bool bIsFullscreen)
    {
        mIsFullscreen = bIsFullscreen;
    }

    uint32_t GetWidth() { return mWidth; }
    uint32_t GetHeight() { return mHeight; }

private:
    static LRESULT CALLBACK StaticWndProc(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam);
    LRESULT HandleMessage(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam);

    [[nodiscard]] bool IsClosedImpl() const { return mIsClosed; }
    [[nodiscard]] bool IsFullscreenImpl() const { return mIsFullscreen; }
    [[nodiscard]] bool IsMouseCapturedImpl() const { return mIsMouseCaptured; }
    [[nodiscard]] int GetFullscreenWidthImpl() const { return FSWidth; }
    [[nodiscard]] int GetFullscreenHeightImpl() const { return FSHeight; }

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
    FWindowCallbacks mCallbacks;
};