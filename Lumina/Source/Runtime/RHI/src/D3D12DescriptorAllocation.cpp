#include "D3D12DescriptorAllocation.h"
#include "D3D12DescriptorAllocatorPage.h"
#include "D3D12DeferredReleaseQueue.h"

#include <cassert>
#include <utility>

FD3D12DescriptorAllocation::FD3D12DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, UINT NumHandles,
                                             UINT DescriptorSize, std::shared_ptr<FD3D12DescriptorAllocatorPage> pPage,
                                             UINT Offset)
        : mCpuHandle(CpuHandle), mNumHandles(NumHandles), mDescriptorSize(DescriptorSize), mpPage(std::move(pPage)), mOffset(Offset)
{
}

FD3D12DescriptorAllocation::~FD3D12DescriptorAllocation()
{
    Free();
}

FD3D12DescriptorAllocation::FD3D12DescriptorAllocation(FD3D12DescriptorAllocation&& Other) noexcept
{
    *this = std::move(Other);
}

FD3D12DescriptorAllocation& FD3D12DescriptorAllocation::operator=(FD3D12DescriptorAllocation&& Other) noexcept
{
    if (this != &Other)
    {
        Free();

        mCpuHandle = Other.mCpuHandle;
        mNumHandles = Other.mNumHandles;
        mDescriptorSize = Other.mDescriptorSize;
        mOffset = Other.mOffset;
        mpPage = std::move(Other.mpPage);

        Other.mCpuHandle.ptr = 0;
        Other.mNumHandles = 0;
        Other.mOffset = 0;
        Other.mpPage.reset();
    }
    return *this;
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12DescriptorAllocation::GetCpuHandle(UINT Offset) const
{
    assert(Offset < mNumHandles);
    return { mCpuHandle.ptr + (static_cast<SIZE_T>(mDescriptorSize) * Offset) };
}

void FD3D12DescriptorAllocation::Free()
{
    if (IsValid() && mpPage)
    {
        auto pPage = mpPage;
        UINT Offset = mOffset;
        UINT Num = mNumHandles;

        FD3D12DeferredReleaseQueue::Enqueue([pPage, Offset, Num]()
        {
            pPage->Free(Offset, Num);
        });

        mCpuHandle.ptr = 0;
        mNumHandles = 0;
        mOffset = 0;
        mpPage.reset();
    }
}
