#include "D3D12DescriptorAllocatorPage.h"
#include "D3D12Device.h"

#include <cassert>

FD3D12DescriptorAllocatorPage::FD3D12DescriptorAllocatorPage(const FD3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                                   UINT NumDescriptors)
        : mHeapType(Type), mNumDescriptorsInHeap(NumDescriptors), mNumFreeHandles(NumDescriptors)
{
    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
    HeapDesc.Type = Type;
    HeapDesc.NumDescriptors = mNumDescriptorsInHeap;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HeapDesc.NodeMask = 0;

    HRESULT HResult = pDevice->GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&mpDescriptorHeap));
    assert(SUCCEEDED(HResult) && "Failed to create Descriptor Allocator Page Heap");

    mFreeList[0] = mNumDescriptorsInHeap;
    mBaseCpuHandle = mpDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    mDescriptorSize = pDevice->GetDevice()->GetDescriptorHandleIncrementSize(mHeapType);
}

bool FD3D12DescriptorAllocatorPage::HasSpace(UINT NumDescriptors) const
{
    return mNumFreeHandles >= NumDescriptors;
}

FD3D12DescriptorAllocation FD3D12DescriptorAllocatorPage::Allocate(UINT NumDescriptors)
{
    std::lock_guard<std::mutex> Lock(mPageMutex);

    if (!HasSpace(NumDescriptors)) { return {}; }

    // Traverse the free list to find the first block that fits
    for (auto it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        const UINT Offset = it->first;
        const UINT Size = it->second;

        if (Size >= NumDescriptors)
        {
            mFreeList.erase(it);

            // If the block is larger than needed, put the remainder back
            if (Size > NumDescriptors)
            {
                mFreeList[Offset + NumDescriptors] = Size - NumDescriptors;
            }

            mNumFreeHandles -= NumDescriptors;
            D3D12_CPU_DESCRIPTOR_HANDLE Handle = { mBaseCpuHandle.ptr + static_cast<SIZE_T>(Offset) * mDescriptorSize };

            return {Handle, NumDescriptors, mDescriptorSize, shared_from_this(), Offset};
        }
    }

    return {};
}

void FD3D12DescriptorAllocatorPage::Free(UINT Offset, UINT NumDescriptors)
{
    std::lock_guard<std::mutex> Lock(mPageMutex);

    // Find the next block in the free list whose offset is greater than the block being freed
    auto NextBlockIt = mFreeList.upper_bound(Offset);
    auto PrevBlockIt = NextBlockIt;

    bool bMergeWithPreviousBlock = false;
    if (NextBlockIt != mFreeList.begin())
    {
        --PrevBlockIt;
        // Check if the previous block exactly touches the start of the block being freed
        if (PrevBlockIt->first + PrevBlockIt->second == Offset)
        {
            bMergeWithPreviousBlock = true;
        }
    }

    bool bMergeWithNextBlock = false;
    if (NextBlockIt != mFreeList.end())
    {
        if (Offset + NumDescriptors == NextBlockIt->first)
        {
            bMergeWithNextBlock = true;
        }
    }

    // Perform Block Coalescing (Merging) to prevent fragmentation
    if (bMergeWithPreviousBlock && bMergeWithNextBlock)
    {
        // Merge both: Add current and next size to previous, remove next
        PrevBlockIt->second += NumDescriptors + NextBlockIt->second;
        mFreeList.erase(NextBlockIt);
    }
    else if (bMergeWithPreviousBlock)
    {
        // Merge with previous only
        PrevBlockIt->second += NumDescriptors;
    }
    else if (bMergeWithNextBlock)
    {
        // Merge with next only: Use current offset, combine sizes, remove old next
        mFreeList[Offset] = NumDescriptors + NextBlockIt->second;
        mFreeList.erase(NextBlockIt);
    }
    else
    {
        // No adjacent free blocks, insert as isolated block
        mFreeList[Offset] = NumDescriptors;
    }

    mNumFreeHandles += NumDescriptors;
}
