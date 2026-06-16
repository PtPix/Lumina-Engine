/**
 * @file DescriptorAllocatorPage.h
 * @brief A single ID3D12DescriptorHeap wrapper acting as a memory page.
 *
 * Uses a free-list allocation strategy to sub-allocate variable sized
 * contiguous descriptor blocks from a single non-shader-visible heap.
 */

#pragma once

#include <memory>
#include <d3d12.h>
#include <map>
#include <mutex>
#include <wrl/client.h>

#include "Renderer/D3D12Core/Descriptors/DescriptorAllocation.h"

class FDevice;

class FDescriptorAllocatorPage : public std::enable_shared_from_this<FDescriptorAllocatorPage>
{
public:
    FDescriptorAllocatorPage(const FDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT NumDescriptors);

    [[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return mHeapType; }
    [[nodiscard]] bool HasSpace(UINT NumDescriptors) const;

    FDescriptorAllocation Allocate(UINT NumDescriptors);
    void Free(UINT Offset, UINT NumDescriptors);

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mpDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;

    D3D12_CPU_DESCRIPTOR_HANDLE mBaseCpuHandle{};
    UINT mDescriptorSize = 0;
    UINT mNumDescriptorsInHeap = 0;
    UINT mNumFreeHandles = 0;

    UINT mCurrentOffset = 0;

    std::mutex mPageMutex;
    std::map<UINT, UINT> mFreeList;
};
