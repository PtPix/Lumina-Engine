/**
 * @file D3D12DescriptorAllocation.h
 * @brief RAII wrapper for an allocated block of CPU descriptors.
 *
 * Automatically frees the descriptor block back to its parent page upon destruction.
 */

#pragma once

#include <d3d12.h>
#include <memory>

class FD3D12DescriptorAllocatorPage;

class FD3D12DescriptorAllocation
{
public:
    FD3D12DescriptorAllocation() = default;
    FD3D12DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, UINT NumHandles, UINT DescriptorSize,
        std::shared_ptr<FD3D12DescriptorAllocatorPage> pPage, UINT Offset);
    ~FD3D12DescriptorAllocation();

    FD3D12DescriptorAllocation(const FD3D12DescriptorAllocation&) = delete;
    FD3D12DescriptorAllocation& operator=(const FD3D12DescriptorAllocation&) = delete;

    FD3D12DescriptorAllocation(FD3D12DescriptorAllocation&& Other) noexcept;
    FD3D12DescriptorAllocation& operator=(FD3D12DescriptorAllocation&& Other) noexcept;

    [[nodiscard]] bool IsValid() const { return mCpuHandle.ptr != 0; }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT Offset = 0) const;
    [[nodiscard]] UINT GetNumHandles() const { return mNumHandles; }

    void Free();

private:
    D3D12_CPU_DESCRIPTOR_HANDLE mCpuHandle{};

    UINT mNumHandles = 0;
    UINT mDescriptorSize = 0;
    UINT mOffset = 0;

    std::shared_ptr<FD3D12DescriptorAllocatorPage> mpPage;
};