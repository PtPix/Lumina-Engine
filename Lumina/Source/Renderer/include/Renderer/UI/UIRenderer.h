#pragma once
#include <windows.h>

class FCommandContext;

class UIRenderer
{
public:
    static void Initialize(HWND Hwnd);
    static void Shutdown();
    static void BeginFrame();
    static void Render(FCommandContext* pCommandContext);
    static bool IsInitialized() { return mbInitialized; };

private:
    static bool mbInitialized;
};