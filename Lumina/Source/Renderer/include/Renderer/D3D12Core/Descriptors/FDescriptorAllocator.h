#pragma once

#include <d3d12.h>
#include <mutex>
#include <vector>

#include "FDescriptorAllocation.h"

class FDevice;

class FDescriptorAllocator
{
public:
    FDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT DescriptorsPerPage = 256);

    void Initialize(FDevice* pDevice);

    FDescriptorAllocation Allocate(UINT NumDescriptors = 1);

private:
    std::shared_ptr<FDescriptorAllocatorPage> CreateAllocatorPage();

private:
    FDevice* mpDevice = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
    UINT mDescriptorsPerPage;

    std::vector<std::shared_ptr<FDescriptorAllocatorPage>> mHeapPool;
    std::mutex mAllocationMutex;
};