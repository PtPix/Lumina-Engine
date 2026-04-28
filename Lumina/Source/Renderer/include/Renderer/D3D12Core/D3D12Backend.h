#pragma once

#include <windows.h>
#include <cstdint>
#include <d3d12.h>
#include <vector>

#include "Core/FSwapChain.h"
#include "Descriptors/FBindlessDescriptorHeap.h"

class FDevice;
class FCommandQueue;
class FDescriptorAllocator;
class FCommandContext;
class FSwapChain;

class D3D12Backend
{
public:
    static bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    static void Shutdown();
    static void OnResize(uint32_t Width, uint32_t Height) {}
    static void FlushGPU();

    static void BeginFrame();
    static void EndFrameAndPresent();

    // Getters
    static FDevice* GetDevice() { return &mDevice; }
    static FSwapChain* GetSwapChain() { return &mSwapChain; }
    static D3D12MA::Allocator* GetAllocator() { return mpAllocator; }

    static FCommandQueue* GetGraphicsQueue() { return &mGraphicsQueue; }
    static FCommandQueue* GetComputeQueue() { return &mComputeQueue; }
    static FCommandQueue* GetCopyQueue() { return &mCopyQueue; }

    static FDescriptorAllocator* GetSrvUavCbvAllocator() { return &mSrvUavCbvAllocator; }
    static FDescriptorAllocator* GetRtvAllocator() { return &mRtvAllocator; }
    static FDescriptorAllocator* GetDsvAllocator() { return &mDsvAllocator; }

    static FBindlessDescriptorHeap* GetBindlessDescriptorHeap() { return &mBindlessDescriptorHeap; }

    static D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTV() { return mSwapChain.GetCurrentBackBufferRTVHandle(); }
    static GpuResource* GetCurrentBackBufferResource() { return mSwapChain.GetCurrentRenderTargetResource(); }

    static FCommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT);
    static void FreeContext(FCommandContext* pContext);

private:
    // Core
    static FDevice mDevice;
    static FSwapChain mSwapChain;
    static D3D12MA::Allocator* mpAllocator;

    // CommandQueue
    static FCommandQueue mGraphicsQueue;
    static FCommandQueue mComputeQueue;
    static FCommandQueue mCopyQueue;

    // Descriptor
    // CPU
    static FDescriptorAllocator mSrvUavCbvAllocator;
    static FDescriptorAllocator mRtvAllocator;
    static FDescriptorAllocator mDsvAllocator;
    // GPU
    static FBindlessDescriptorHeap mBindlessDescriptorHeap;

    // Command Context
    static std::vector<FCommandContext*> mContextPool[4];
    static std::vector<FCommandContext*> mAvailableContextPool[4];
};
