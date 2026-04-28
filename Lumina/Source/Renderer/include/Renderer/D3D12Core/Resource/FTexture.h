#pragma once
#include <string>
#include "D3D12MemAlloc.h"
#include "GpuResource.h"
#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocation.h"

class FDevice;

class FTexture : public GpuResource
{
public:
    FTexture() = default;
    ~FTexture() override { Destroy(); }

    FTexture(FTexture&& Other) noexcept;
    FTexture& operator=(FTexture&& Other) noexcept;

    // 为了绝对安全，显式禁用拷贝语义 (Copy Semantics)
    FTexture(const FTexture&) = delete;
    FTexture& operator=(const FTexture&) = delete;

    bool Create(FDevice* pDevice, D3D12MA::Allocator* pAllocator,
                UINT Width, UINT Height, DXGI_FORMAT Format,
                D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE,
                D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON,
                const D3D12_CLEAR_VALUE* pClearValue = nullptr,
                const std::wstring& Name = L"Texture");

    void CreateFromSwapChain(FDevice* pDevice, ID3D12Resource* pResource);
    void Create2D(FDevice* pDevice, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12MA::Allocation* pAllocation, ID3D12Resource* pResource);
    void Destroy();

    // Getter
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return mRTV.GetCpuHandle(); }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return mSRV.GetCpuHandle(); }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return mDSV.GetCpuHandle(); }
private:
    FDescriptorAllocation mRTV;
    FDescriptorAllocation mSRV;
    FDescriptorAllocation mDSV;

    UINT mWidth = 0;
    UINT mHeight = 0;
    DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;

    D3D12MA::Allocation* mpAllocation = nullptr;
};
