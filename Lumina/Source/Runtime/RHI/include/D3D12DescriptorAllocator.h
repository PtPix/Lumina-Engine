/**
 * @file D3D12DescriptorAllocator.h
 * @brief CPU Descriptor Allocator.
 *
 * Manages a pool of descriptor pages (FDescriptorAllocatorPage) for a specific heap type.
 * Handles dynamic allocation of contiguous CPU descriptor blocks.
 */

#pragma once

#include <d3d12.h>
#include <mutex>
#include <vector>
#include <memory>

#include "D3D12DescriptorAllocation.h"

class FD3D12Device;
class FD3D12DescriptorAllocatorPage;

class FD3D12DescriptorAllocator
{
public:
    FD3D12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT DescriptorsPerPage = 256);

    void Initialize(FD3D12Device* pDevice);

    FD3D12DescriptorAllocation Allocate(UINT NumDescriptors = 1);

private:
    std::shared_ptr<FD3D12DescriptorAllocatorPage> CreateAllocatorPage();

private:
    FD3D12Device* mpDevice = nullptr;

    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
    UINT mDescriptorsPerPage;

    std::vector<std::shared_ptr<FD3D12DescriptorAllocatorPage>> mHeapPool;
    std::mutex mAllocationMutex;
};