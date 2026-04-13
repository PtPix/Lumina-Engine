#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocation.h"

#include <cassert>

FDescriptorAllocation::FDescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, UINT NumHandles,
                                             UINT DescriptorSize, std::shared_ptr<FDescriptorAllocatorPage> pPage)
        : mCpuHandle(CpuHandle), mNumHandles(NumHandles), mDescriptorSize(DescriptorSize), mpPage(pPage)
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
        mpPage = std::move(Other.mpPage);

        Other.mCpuHandle.ptr = 0;
        Other.mNumHandles = 0;
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
        mCpuHandle.ptr = 0;
        mNumHandles = 0;
        mpPage.reset();
    }
}
