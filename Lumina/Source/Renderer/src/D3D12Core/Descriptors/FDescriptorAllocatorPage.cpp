#include <cassert>

#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocatorPage.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

FDescriptorAllocatorPage::FDescriptorAllocatorPage(const FDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                                   UINT NumDescriptors)
        : mHeapType(Type), mNumDescriptorsInHeap(NumDescriptors), mNumFreeHandles(NumDescriptors)
{
    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
    HeapDesc.Type = Type;
    HeapDesc.NumDescriptors = mNumDescriptorsInHeap;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HeapDesc.NodeMask = 0;

    HRESULT HResult = pDevice->GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&mpDescriptorHeap));
    assert(SUCCEEDED(HResult));

    mNumFreeHandles = NumDescriptors;
    mFreeList[0] = mNumDescriptorsInHeap;
    mBaseCpuHandle = mpDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    mDescriptorSize = pDevice->GetDevice()->GetDescriptorHandleIncrementSize(mHeapType);
}

bool FDescriptorAllocatorPage::HasSpace(UINT NumDescriptors) const
{
    return mNumFreeHandles >= NumDescriptors;
}

FDescriptorAllocation FDescriptorAllocatorPage::Allocate(UINT NumDescriptors)
{
    std::lock_guard<std::mutex> Lock(mPageMutex);

    // Fast Path
    if (!HasSpace(NumDescriptors)) { return {}; }

    // Check Free List
    for (auto it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        UINT Offset = it->first;
        UINT Size = it->second;

        if (Size >= NumDescriptors)
        {
            mFreeList.erase(it);
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

void FDescriptorAllocatorPage::Free(UINT Offset, UINT NumDescriptors)
{
    std::lock_guard<std::mutex> Lock(mPageMutex);

    mFreeList[Offset] = NumDescriptors;
    mNumFreeHandles += NumDescriptors;
}
