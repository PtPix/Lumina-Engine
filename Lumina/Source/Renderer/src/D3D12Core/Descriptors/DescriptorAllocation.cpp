#include "Renderer/D3D12Core/Descriptors/DescriptorAllocation.h"
#include "Renderer/D3D12Core/Descriptors/DescriptorAllocatorPage.h"
#include "Renderer/D3D12Core/Core/DeferredReleaseQueue.h"

#include <cassert>
#include <utility>

FDescriptorAllocation::FDescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, UINT NumHandles,
                                             UINT DescriptorSize, std::shared_ptr<FDescriptorAllocatorPage> pPage,
                                             UINT Offset)
        : mCpuHandle(CpuHandle), mNumHandles(NumHandles), mDescriptorSize(DescriptorSize), mpPage(std::move(pPage)), mOffset(Offset)
{
}

FDescriptorAllocation::~FDescriptorAllocation()
{
    Free();
}

FDescriptorAllocation::FDescriptorAllocation(FDescriptorAllocation&& Other) noexcept
{
    *this = std::move(Other);
}

FDescriptorAllocation& FDescriptorAllocation::operator=(FDescriptorAllocation&& Other) noexcept
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

D3D12_CPU_DESCRIPTOR_HANDLE FDescriptorAllocation::GetCpuHandle(UINT Offset) const
{
    assert(Offset < mNumHandles);
    return { mCpuHandle.ptr + (static_cast<SIZE_T>(mDescriptorSize) * Offset) };
}

void FDescriptorAllocation::Free()
{
    if (IsValid() && mpPage)
    {
        auto pPage = mpPage;
        UINT Offset = mOffset;
        UINT Num = mNumHandles;

        FDeferredReleaseQueue::Enqueue([pPage, Offset, Num]()
        {
            pPage->Free(Offset, Num);
        });

        mCpuHandle.ptr = 0;
        mNumHandles = 0;
        mOffset = 0;
        mpPage.reset();
    }
}
