#pragma once

#include <windows.h>
#include <cstdint>
#include <d3d12.h>
#include <memory>

#include "Core/FDevice.h"
#include "Core/FSwapChain.h"

class FDevice;
class FCommandQueue;
class FDescriptorAllocator;
class FCommandContext;
class FSwapChain;
class FBindlessDescriptorHeap;
class GpuResource;

namespace D3D12MA { class Allocator; }

class D3D12Backend
{
public:
    static bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    static void Shutdown();
    static void OnResize(uint32_t Width, uint32_t Height);
    static void FlushGPU();

    static void BeginFrame();
    static void EndFrameAndPresent();

    // Getters
    static FDevice* GetDevice() { return mpDevice.get(); }
    static FSwapChain* GetSwapChain() { return mpSwapChain.get(); }
    static D3D12MA::Allocator* GetAllocator() { return mpDevice->GetAllocator(); }

    static FCommandQueue* GetGraphicsQueue() { return mpDevice->GetGraphicsCommandQueue(); }
    static FCommandQueue* GetComputeQueue() { return mpDevice->GetComputeCommandQueue(); }
    static FCommandQueue* GetCopyQueue() { return mpDevice->GetCopyCommandQueue(); }

    static FDescriptorAllocator* GetSrvUavCbvAllocator() { return mpDevice->GetSRVAllocator(); }
    static FDescriptorAllocator* GetRtvAllocator() { return mpDevice->GetRTVAllocator(); }
    static FDescriptorAllocator* GetDsvAllocator() { return mpDevice->GetDSVAllocator(); }

    static FBindlessDescriptorHeap* GetBindlessDescriptorHeap() { return mpDevice->GetBindlessDescriptorHeap(); }

    static D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTV() { return mpSwapChain->GetCurrentBackBufferRTVHandle(); }
    static GpuResource* GetCurrentBackBufferResource() { return mpSwapChain->GetCurrentRenderTargetResource(); }

    static FCommandContext* AllocateContext();
    static uint64_t ExecuteGraphicsContext(FCommandContext* pCommandContext);

private:
    static std::unique_ptr<FDevice> mpDevice;
    static std::unique_ptr<FSwapChain> mpSwapChain;
};
