#pragma once

#include "Device.h"
#include "CommandQueue.h"
#include "D3D12MemAlloc.h"
#include "SwapChain.h"
#include "Common.h"
#include "Renderer/RenderCore/Buffer.h"
#include "Renderer/RenderCore/ResourceHeaps.h"

#include <cstdint>
#include <windows.h>

// Hold GPU infras
class GraphicsDevice
{
public:
    bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    void Destroy();

    ID3D12GraphicsCommandList* BeginFrame();
    void EndFrameAndPresent();

    void OnResize(uint32_t Width, uint32_t Height);

public:
    HWND GetWindowHandle() const { return mHwnd; }
    const Device& GetDevice() const { return mDevice; }
    D3D12MA::Allocator* GetAllocator() const { return mpAllocator; }

    CommandQueue& GetGraphicsQueue() { return mCommandQueues[GRAPHICS]; }
    SwapChain& GetSwapChain() { return mSwapChain; }

    UploadHeap& GetUploadHeap() { return mHeapUpload; }
    DynamicUploadHeap& GetDynamicUploadHeap() { return mDynamicUploadHeaps[mFrameIndex]; }

    StaticBufferHeap& GetVertexBufferHeap() { return mStaticHeapVertexBuffer; }
    StaticBufferHeap& GetIndexBufferHeap() { return mStaticHeapIndexBuffer; }
    StaticBufferHeap& GetConstantBufferHeap() { return mStaticHeapConstantBuffer; }

    StaticResourceViewHeap& GetCbvSrvUavHeap() { return mHeapCBV_SRV_UAV; }
    StaticResourceViewHeap& GetRTVHeap() { return mHeapRTV; }
    StaticResourceViewHeap& GetDsvHeap() { return mHeapDSV; }

private:
    D3D12_COMMAND_LIST_TYPE GetDX12CommandListType(ECommandQueueType Type);

private:
    HWND mHwnd = nullptr;
    uint32_t mFrameIndex = 0;

    Device mDevice;
    D3D12MA::Allocator* mpAllocator = nullptr;
    CommandQueue mCommandQueues[NUM_COMMAND_QUEUE_TYPES];
    SwapChain mSwapChain;

    StaticResourceViewHeap mHeapDSV;
    StaticResourceViewHeap mHeapCBV_SRV_UAV;
    StaticResourceViewHeap mHeapRTV;

    StaticBufferHeap mStaticHeapVertexBuffer;
    StaticBufferHeap mStaticHeapIndexBuffer;
    StaticBufferHeap mStaticHeapConstantBuffer;
    UploadHeap mHeapUpload;

    DynamicUploadHeap mDynamicUploadHeaps[NUM_SWAPCHAIN_BACKBUFFER];

    ID3D12CommandAllocator* mpCommandAllocators[NUM_COMMAND_QUEUE_TYPES][NUM_SWAPCHAIN_BACKBUFFER] = {};
    ID3D12CommandList* mpCommandLists[NUM_COMMAND_QUEUE_TYPES][NUM_SWAPCHAIN_BACKBUFFER] = {};
    uint8_t mClosedCommandLists[NUM_COMMAND_QUEUE_TYPES][NUM_SWAPCHAIN_BACKBUFFER] = {};

};