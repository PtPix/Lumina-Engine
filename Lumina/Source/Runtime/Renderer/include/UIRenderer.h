#pragma once

#include <windows.h>
#include <d3d12.h>
#include <cstdint>

#include "ImGUI/imgui.h"

class FD3D12Device;
class FD3D12CommandContext;

class UIRenderer
{
public:
    static constexpr uint32_t ViewportTextureSlot = 1;

    static ImTextureID GetTextureID(uint32_t Slot);
    static void CopySRVToSlot(FD3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcSRV, uint32_t Slot);

    static void Initialize(HWND Hwnd, FD3D12Device* pDevice);
    static void Shutdown();
    static void BeginFrame();
    static void Render(FD3D12CommandContext* pCommandContext, D3D12_CPU_DESCRIPTOR_HANDLE Rtv);
    static bool IsInitialized() { return mbInitialized; };
    static bool ProcessWin32Message(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam, LRESULT& OutResult);
private:
    static bool mbInitialized;
};