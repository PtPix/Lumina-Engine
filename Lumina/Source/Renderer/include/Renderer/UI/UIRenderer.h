#pragma once
#include <windows.h>

class FDevice;
class FCommandContext;

class UIRenderer
{
public:
    static void Initialize(HWND Hwnd, FDevice* pDevice);
    static void Shutdown();
    static void BeginFrame();
    static void Render(FCommandContext* pCommandContext, D3D12_CPU_DESCRIPTOR_HANDLE Rtv);
    static bool IsInitialized() { return mbInitialized; };
    static bool ProcessWin32Message(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam, LRESULT& OutResult);
private:
    static bool mbInitialized;
};