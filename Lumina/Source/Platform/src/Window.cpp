#include "Platform/Window.h"

#include "Logger/Logger.h"
#include "Platform/InputState.h"

static std::vector<std::string> split(const char* s, char c)
{
    std::vector<std::string> result;
    do
    {
        const char* begin = s;

        if (*begin == c || *begin == '\0')
            continue;	// skip delimiter character

        while (*s != c && *s)
            s++;	// iterate until delimiter is found

        result.push_back(std::string(begin, s));

    } while (*s++);
    return result;
}

static std::vector<std::string> split(const std::string& str, char c)
{
    return split(str.c_str(), c);
}

static std::vector<std::string> split(std::string_view s, const std::vector<char>& delimiters)
{
    std::vector<std::string> result;
    const char* ps = s.data();
    auto IsDelimiter = [&delimiters](const char c)
    {
        return std::find(delimiters.begin(), delimiters.end(), c) != delimiters.end();
    };

    do
    {
        const char* begin = ps;

        if (IsDelimiter(*begin) || (*begin == '\0'))
            continue;	// skip delimiter characters

        while (!IsDelimiter(*ps) && *ps)
            ps++;	// iterate until delimiter is found or string has ended

        result.push_back(std::string(begin, ps));

    } while (*ps++);
    return result;
}

static RECT CenterScreen(const RECT& screenRect, const RECT& wndRect)
{
    RECT centered = {};

    const int szWndX = wndRect.right - wndRect.left;
    const int szWndY = wndRect.bottom - wndRect.top;
    const int offsetX = (screenRect.right - screenRect.left - szWndX) / 2;
    const int offsetY = (screenRect.bottom - screenRect.top - szWndY) / 2;

    centered.left = screenRect.left + offsetX;
    centered.right = centered.left + szWndX;
    centered.top = screenRect.top + offsetY;
    centered.bottom = centered.top + szWndY;

    return centered;
}

static RECT GetScreenRectOnPreferredDisplay(const RECT& PreferredRect, int PreferredDisplayIndex, bool* bMonitorFound)
{
    struct MonitorEnumCallbackParams
    {
        int PreferredMonitorIndex = 0;
        const RECT* pRectOriginal = nullptr;
        RECT* pRectNew = nullptr;
        RECT RectDefault{};
    };
    RECT PreferredScreenRect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT };
    MonitorEnumCallbackParams Params = {};
    Params.PreferredMonitorIndex = PreferredDisplayIndex;
    Params.pRectOriginal = &PreferredRect;
    Params.pRectNew = &PreferredScreenRect;

    auto fnCallbackMonitorFound = [](HMONITOR Arg1, HDC Arg2, LPRECT Arg3, LPARAM Arg4) -> BOOL
    {
        BOOL b = TRUE;
        MonitorEnumCallbackParams* pParam = (MonitorEnumCallbackParams*)Arg4;

        MONITORINFOEX MonitorInfo = {};
        MonitorInfo.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(Arg1, &MonitorInfo);

        std::string MonitorName(MonitorInfo.szDevice);
        MonitorName = split(MonitorName, { '/', '\\', '.' })[0];
        std::string StrMonitorIndex = MonitorName.substr(MonitorName.size() - 1);
        const int MonitorIndex = std::atoi(StrMonitorIndex.c_str()) - 1;

        if (MonitorIndex == pParam->PreferredMonitorIndex)
        {
            *pParam->pRectNew = *Arg3;
        }
        if (MonitorIndex == 0)
        {
            pParam->RectDefault = *Arg3;
        }
        return b;
    };

    EnumDisplayMonitors(nullptr, nullptr, fnCallbackMonitorFound, (LPARAM)&Params);
    const bool bPreferredDisplayNotFound =
        (PreferredScreenRect.right == PreferredScreenRect.left
    && PreferredScreenRect.left == PreferredScreenRect.top
    && PreferredScreenRect.top == PreferredScreenRect.bottom)
    && (PreferredScreenRect.right == CW_USEDEFAULT);

    *bMonitorFound = !bPreferredDisplayNotFound;

    return bPreferredDisplayNotFound ? Params.RectDefault : CenterScreen(PreferredScreenRect, PreferredRect);
}

// Window interfaces
FWindow::FWindow(const wchar_t* Title, const FWindowDesc& InitParameters)
    : mWidth(InitParameters.Width)
    , mHeight(InitParameters.Height)
    , mIsFullscreen(InitParameters.bFullScreen)
    , mCallbacks(InitParameters.Callbacks)
{
    UINT FlagWindowStyle = WS_OVERLAPPEDWINDOW;

    ::RECT Rect;
    ::SetRect(&Rect, 0, 0, mWidth, mHeight);
    ::AdjustWindowRect(&Rect, FlagWindowStyle, FALSE);

    HWND HwndParent = nullptr;

    mWindowClass = std::make_unique<WindowClass>(
        L"LuminaWindowClass",
        InitParameters.hInstance,
        &FWindow::StaticWndProc
        );

    bool bPreferredDisplayFound = false;
    RECT PreferredScreenRect = GetScreenRectOnPreferredDisplay(Rect, InitParameters.PreferredDisplay, &bPreferredDisplayFound);

    this->FSWidth = PreferredScreenRect.right - PreferredScreenRect.left;
    this->FSHeight = PreferredScreenRect.bottom - PreferredScreenRect.top;

    mHwnd = CreateWindowExW(0,
        mWindowClass->GetName(),
        Title,
        FlagWindowStyle,
        bPreferredDisplayFound ? PreferredScreenRect.left : CW_USEDEFAULT,
        bPreferredDisplayFound ? PreferredScreenRect.top : CW_USEDEFAULT,
        Rect.right - Rect.left,
        Rect.bottom - Rect.top,
        HwndParent,
        nullptr,
        InitParameters.hInstance,
        this
        );

    mWindowStyle = FlagWindowStyle;

    ::ShowWindow(mHwnd, InitParameters.iShowCommand);

    ::SetWindowLongPtr(mHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if (!bPreferredDisplayFound)
    {
        LUMINA_LOG_WARNING(Platform, "Window(): Couldn't find the preferred display %d", InitParameters.PreferredDisplay);
    }
}

HWND FWindow::GetHWND() const
{
    return mHwnd;
}

void FWindow::Show()
{
    ::ShowWindow(mHwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(mHwnd);
}

void FWindow::Minimize()
{
    ::ShowWindow(mHwnd, SW_MINIMIZE);
}

void FWindow::ToggleWindowedFullScreen()
{
}

void FWindow::Close()
{
    LUMINA_LOG_INFO(Platform, "Window:: Closing<%p>", this->mHwnd);
    this->mIsClosed = true;
    ::ShowWindow(mHwnd, FALSE);
    ::DestroyWindow(this->mHwnd);
}

void FWindow::SetMouseCapture(bool bCapture)
{
    mIsMouseCaptured = bCapture;
    if (bCapture)
    {
        RECT RectClip;
        GetWindowRect(mHwnd, &RectClip);

        // Keep the cursor inside pixel area
        constexpr int PX_OFFSET = 15;
        constexpr int PX_WND_TITLE_OFFSET = 30;
        RectClip.left += PX_OFFSET;
        RectClip.right -= PX_OFFSET;
        RectClip.top += PX_OFFSET + PX_WND_TITLE_OFFSET;
        RectClip.bottom -= PX_OFFSET;

        int HResult = ShowCursor(FALSE);
        while (HResult >= 0)
        {
            HResult = ShowCursor(FALSE);
        }

        ClipCursor(&RectClip);
        SetForegroundWindow(mHwnd);
        SetFocus(mHwnd);
    }
    else
    {
        ClipCursor(nullptr);
        while (ShowCursor(TRUE) <= 0);
        SetForegroundWindow(nullptr);
    }
}

LRESULT FWindow::StaticWndProc(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam)
{
    FWindow* pWindow = nullptr;

    if (Message == WM_NCCREATE)
    {
        auto* Create = reinterpret_cast<CREATESTRUCTW*>(LParam);
        pWindow = static_cast<FWindow*>(Create->lpCreateParams);
        SetWindowLongPtrW(Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
        pWindow->mHwnd = Hwnd;
    }
    else
    {
        pWindow = reinterpret_cast<FWindow*>(GetWindowLongPtrW(Hwnd, GWLP_USERDATA));
    }

    if (!pWindow)
    {
        return DefWindowProcW(Hwnd, Message, WParam, LParam);
    }

    return pWindow->HandleMessage(Hwnd, Message, WParam, LParam);
}

LRESULT FWindow::HandleMessage(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    if (mCallbacks.fnNativeMessageHandler)
    {
        FWin32Message Msg{ Hwnd, Message, WParam, LParam };
        if (mCallbacks.fnNativeMessageHandler(Msg, Result))
        {
            return Result;
        }
    }

    Input::ProcessMessage(Message, WParam, LParam);

    switch (Message)
    {
    case WM_SIZE:
        if (WParam == SIZE_MINIMIZED)
        {
            if (mCallbacks.fnOnMinimize) mCallbacks.fnOnMinimize(*this);
        }
        else
        {
            mWidth = LOWORD(LParam);
            mHeight = HIWORD(LParam);

            if (mCallbacks.fnOnResize)
            {
                mCallbacks.fnOnResize(*this, mWidth, mHeight);
            }
        }
        return 0;

    case WM_SETFOCUS:
        if (mCallbacks.fnOnFocus) mCallbacks.fnOnFocus(*this);
        return 0;

    case WM_KILLFOCUS:
        if (mCallbacks.fnOnLoseFocus) mCallbacks.fnOnLoseFocus(*this);
        return 0;

    case WM_CLOSE:
        if (mCallbacks.fnOnDestroy) mCallbacks.fnOnDestroy(*this);
        Close();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(Hwnd, Message, WParam, LParam);
    }
}

// WindowClass interface
WindowClass::WindowClass(const wchar_t* Name, HINSTANCE hInstance, WNDPROC Procedure)
{
    wcsncpy_s(mName, Name, sizeof(mName));
    ::WNDCLASSEXW WC = {};

    // Register the window class to the main window
    WC.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    WC.lpfnWndProc = Procedure;
    WC.cbClsExtra = 0;
    WC.cbWndExtra = 0;
    WC.hInstance = hInstance;
    WC.hIcon = LoadIcon(nullptr, MAKEINTRESOURCE(100));
    if (WC.hIcon == nullptr)
    {
        DWORD DWord = GetLastError();
        LUMINA_LOG_WARNING(Platform, "Couldn't load icon for window: 0x%x", DWord);
    }
    WC.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WC.hbrBackground = nullptr;
    WC.lpszMenuName = nullptr;
    WC.lpszClassName = mName;
    WC.cbSize = sizeof(WNDCLASSEX);

    ::RegisterClassExW(&WC);
}

WindowClass::~WindowClass()
{
    ::UnregisterClassW(mName, (HINSTANCE)::GetModuleHandle(nullptr));
}
