#pragma once

#include "D3D12MemAlloc.h"
#include "GpuResource.h"
#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocation.h"

class FDevice;

class Texture : public GpuResource
{
public:
    Texture() = default;

    void CreateFromSwapChain(FDevice* pDevice, ID3D12Resource* pResource);
    void Create2D(FDevice* pDevice, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12MA::Allocation* pAllocation, ID3D12Resource* pResource);
    void Destroy();

    // Getter
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return mRTV.GetCpuHandle(); }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return mSRV.GetCpuHandle(); }

private:
    FDescriptorAllocation mRTV;
    FDescriptorAllocation mSRV;
    FDescriptorAllocation mDSV;

    UINT mWidth = 0;
    UINT mHeight = 0;
    DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;

    D3D12MA::Allocation* mpAllocation = nullptr;
};
