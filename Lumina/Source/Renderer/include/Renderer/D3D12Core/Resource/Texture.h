#pragma once

#include "GpuResource.h"
#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocation.h"

class FDevice;

class Texture : public GpuResource
{
public:
    Texture() = default;

    void CreateFromSwapChain(FDevice* pDevice, ID3D12Resource* pResource);

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
};
