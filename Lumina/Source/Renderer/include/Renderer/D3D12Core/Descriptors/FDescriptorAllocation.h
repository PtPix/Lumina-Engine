#pragma once

#include <d3d12.h>
#include <memory>

class FDescriptorAllocatorPage;

class FDescriptorAllocation
{
public:
    FDescriptorAllocation() = default;
    FDescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, UINT NumHandles, UINT DescriptorSize,
        std::shared_ptr<FDescriptorAllocatorPage> pPage, UINT Offset);
    ~FDescriptorAllocation();

    FDescriptorAllocation(const FDescriptorAllocation&) = delete;
    FDescriptorAllocation& operator=(const FDescriptorAllocation&) = delete;
    FDescriptorAllocation(FDescriptorAllocation&& Other) noexcept;
    FDescriptorAllocation& operator=(FDescriptorAllocation&& Other) noexcept;

    [[nodiscard]] bool IsValid() const { return mCpuHandle.ptr != 0; }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT Offset = 0) const;
    [[nodiscard]] UINT GetNumHandles() const { return mNumHandles; }

    void Free();

private:
    D3D12_CPU_DESCRIPTOR_HANDLE mCpuHandle{};
    UINT mNumHandles = 0;
    UINT mDescriptorSize = 0;

    std::shared_ptr<FDescriptorAllocatorPage> mpPage;
    UINT mOffset = 0;
};