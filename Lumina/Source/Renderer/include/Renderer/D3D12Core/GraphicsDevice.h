#pragma once

#include "Core/FDevice.h"
#include "Core/FCommandQueue.h"
#include "D3D12MemAlloc.h"
#include "Core/FSwapChain.h"
#include "Common.h"
#include "Buffer.h"
#include "ResourceHeaps.h"

#include <cstdint>
#include <windows.h>

#include "Core/FCommandContext.h"

// Hold GPU infras
class GraphicsDevice
{
public:
    bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    void Destroy();

    FCommandContext* BeginFrame();
    void EndFrameAndPresent();

    void OnResize(uint32_t Width, uint32_t Height);

public:
    HWND GetWindowHandle() const { return mHwnd; }
    FDevice& GetDevice() { return mDevice; }
    D3D12MA::Allocator* GetAllocator() const { return mpAllocator; }

    FCommandQueue& GetGraphicsQueue() { return mCommandQueues[GRAPHICS]; }
    FSwapChain& GetSwapChain() { return mSwapChain; }

    UploadHeap& GetUploadHeap() { return mHeapUpload; }
    DynamicUploadHeap& GetDynamicUploadHeap() { return mDynamicUploadHeaps[mFrameIndex]; }

    StaticBufferHeap& GetVertexBufferHeap() { return mStaticHeapVertexBuffer; }
    StaticBufferHeap& GetIndexBufferHeap() { return mStaticHeapIndexBuffer; }
    StaticBufferHeap& GetConstantBufferHeap() { return mStaticHeapConstantBuffer; }

    StaticResourceViewHeap& GetCbvSrvUavHeap() { return mHeapCBV_SRV_UAV; }
    StaticResourceViewHeap& GetRTVHeap() { return mHeapRTV; }
    StaticResourceViewHeap& GetDsvHeap() { return mHeapDSV; }
    StaticResourceViewHeap& GetImGuiSrvHeap() { return mHeapImGuiSRV; }

    FCommandContext& GetGraphicsContext() { return mGraphicsCommandContext[mFrameIndex]; }

private:
    static D3D12_COMMAND_LIST_TYPE GetDX12CommandListType(ECommandQueueType Type);

private:
    HWND mHwnd = nullptr;
    uint32_t mFrameIndex = 0;

    FDevice mDevice;
    D3D12MA::Allocator* mpAllocator = nullptr;
    FCommandQueue mCommandQueues[NUM_COMMAND_QUEUE_TYPES];
    FSwapChain mSwapChain;

    StaticResourceViewHeap mHeapDSV;
    StaticResourceViewHeap mHeapCBV_SRV_UAV;
    StaticResourceViewHeap mHeapRTV;
    StaticResourceViewHeap mHeapImGuiSRV;  // ImGui 专用 shader-visible SRV heap

    StaticBufferHeap mStaticHeapVertexBuffer;
    StaticBufferHeap mStaticHeapIndexBuffer;
    StaticBufferHeap mStaticHeapConstantBuffer;
    UploadHeap mHeapUpload;

    DynamicUploadHeap mDynamicUploadHeaps[NUM_SWAPCHAIN_BACKBUFFER];

    FCommandContext mGraphicsCommandContext[NUM_SWAPCHAIN_BACKBUFFER];
    ID3D12CommandAllocator* mpCommandAllocators[NUM_COMMAND_QUEUE_TYPES][NUM_SWAPCHAIN_BACKBUFFER] = {};
    ID3D12CommandList* mpCommandLists[NUM_COMMAND_QUEUE_TYPES][NUM_SWAPCHAIN_BACKBUFFER] = {};
    uint8_t mClosedCommandLists[NUM_COMMAND_QUEUE_TYPES][NUM_SWAPCHAIN_BACKBUFFER] = {};

};