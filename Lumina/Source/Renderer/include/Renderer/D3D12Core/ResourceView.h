#pragma once
#include <cstdint>

#include "D3D12MemAlloc.h"

class ResourceView
{
public:
    [[nodiscard]] uint32_t GetSize() const { return mSize; }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(uint32_t Index = 0) const
    {
        return D3D12_CPU_DESCRIPTOR_HANDLE{ mCpuDescriptorHandle.ptr + static_cast<uint64_t>(Index) * mDescriptorSize };
    }
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(uint32_t Index = 0) const
    {
        return D3D12_GPU_DESCRIPTOR_HANDLE{ mGpuDescriptorHandle.ptr + static_cast<uint64_t>(Index) * mDescriptorSize };
    }

    inline void SetResourceView(uint32_t Size, uint32_t DescriptorSize, D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE GpuDescriptor)
    {
        mSize = Size;
        mDescriptorSize = DescriptorSize;
        mCpuDescriptorHandle = CpuDescriptor;
        mGpuDescriptorHandle = GpuDescriptor;
    }

private:
    uint32_t mSize = 0;
    uint32_t mDescriptorSize = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE mCpuDescriptorHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE mGpuDescriptorHandle = {};
};

using VBV = D3D12_VERTEX_BUFFER_VIEW;
using IBV = D3D12_INDEX_BUFFER_VIEW;
class RTV : public ResourceView {};
class DSV : public ResourceView {};
class CBV_SRV_UAV : public ResourceView {};
class SAMPLER : public ResourceView {};
using SRV = CBV_SRV_UAV;
using CBV = CBV_SRV_UAV;
using UAV = CBV_SRV_UAV;
