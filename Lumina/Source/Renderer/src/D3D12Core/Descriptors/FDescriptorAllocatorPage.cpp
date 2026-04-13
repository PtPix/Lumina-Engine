#include <cassert>

#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocatorPage.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

FDescriptorAllocatorPage::FDescriptorAllocatorPage(FDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type,
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

    mBaseCpuHandle = mpDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    mDescriptorSize = pDevice->GetDevice()->GetDescriptorHandleIncrementSize(mHeapType);
}

bool FDescriptorAllocatorPage::HasSpace(UINT NumDescriptors) const
{
    return mNumFreeHandles >= NumDescriptors;
}

FDescriptorAllocation FDescriptorAllocatorPage::Allocate(UINT NumDescriptors)
{
    if (!HasSpace(NumDescriptors))
    {
        return FDescriptorAllocation();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Handle = { mBaseCpuHandle.ptr + static_cast<SIZE_T>(mCurrentOffset) * mDescriptorSize };

    mCurrentOffset += NumDescriptors;
    mNumFreeHandles -= NumDescriptors;

    return FDescriptorAllocation(Handle, NumDescriptors, mDescriptorSize, shared_from_this());
}

void FDescriptorAllocatorPage::Free(FDescriptorAllocation&& Allocation)
{
}
