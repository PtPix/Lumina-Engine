/**
 * @file D3D12BindlessDescriptorHeap.h
 * @brief DirectX 12 Bindless Descriptor Heap Manager.
 *
 * Manages a single, large, shader-visible descriptor heap for SRV, CBV, and UAVs.
 * Handles slot allocation and thread-safe deferred freeing based on GPU fence completion.
 */

#pragma once

#include <cstdint>
#include <d3d12.h>
#include <mutex>
#include <queue>
#include <vector>
#include <wrl/client.h>

class FD3D12Device;
class FD3D12CommandQueue;

struct FD3D12DeferredFreeSlot
{
    FD3D12CommandQueue* pCommandQueue;
    uint64_t FenceValue;
    uint32_t SlotIndex;
};

class FD3D12BindlessDescriptorHeap
{
public:
    FD3D12BindlessDescriptorHeap() = default;
    ~FD3D12BindlessDescriptorHeap();

    FD3D12BindlessDescriptorHeap(const FD3D12BindlessDescriptorHeap&) = delete;
    FD3D12BindlessDescriptorHeap& operator=(const FD3D12BindlessDescriptorHeap&) = delete;

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    bool Initialize(const FD3D12Device* pDevice, UINT MaxDescriptors = 100000);

    // ------------------------------------------------------------------------
    // Slot Management
    // ------------------------------------------------------------------------
    uint32_t AllocateSlot();
    void FreeSlot(uint32_t Index, FD3D12CommandQueue* pQueue, uint64_t FenceValue);
    void ReleaseStaleSlots();

    // ------------------------------------------------------------------------
    // Descriptor Operations
    // ------------------------------------------------------------------------
    void CopyDescriptor(const FD3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcCpuHandle, uint32_t DestIndex) const;
    void CreateSRVFromCPUHandle(const FD3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcCpuHandle, uint32_t DestIndex) const;

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32_t Index) const;
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint32_t Index) const;
    [[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeap() const { return mpDescriptorHeap.Get(); }

    [[nodiscard]] uint32_t GetAllocatedCount() const { return mCurrentWaterMark; }
    [[nodiscard]] uint32_t GetMaxDescriptors() const { return mMaxDescriptors; }

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mpDescriptorHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE mBaseCpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE mBaseGpuHandle = {};

    UINT mDescriptorSize = 0;
    UINT mMaxDescriptors = 0;

    // Allocation Tracking
    std::mutex mAllocationMutex;
    UINT mCurrentWaterMark = 0;
    std::queue<uint32_t> mFreeSlots;

    // Deferred Freeing
    std::vector<FD3D12DeferredFreeSlot> mDeferredFreeSlots;
};