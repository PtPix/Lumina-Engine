/**
 * @file Texture.h
 * @brief GPU Texture resource wrapper.
 *
 * Manages D3D12 2D Textures and their associated descriptors (RTV, SRV, DSV).
 * Handles ownership of memory allocations and descriptor lifecycle.
 */

#pragma once

#include "Renderer/D3D12Core/Resource/GpuResource.h"
#include "Renderer/D3D12Core/Descriptors/DescriptorAllocation.h"
#include "D3D12MemAlloc.h"

#include <string>

class FDevice;

class FTexture : public GpuResource
{
public:
    FTexture() = default;
    ~FTexture() override { Destroy(); }

    FTexture(const FTexture&) = delete;
    FTexture& operator=(const FTexture&) = delete;

    FTexture(FTexture&& Other) noexcept;
    FTexture& operator=(FTexture&& Other) noexcept;

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    bool Create(FDevice* pDevice, D3D12MA::Allocator* pAllocator,
                UINT Width, UINT Height, DXGI_FORMAT Format,
                D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE,
                D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON,
                const D3D12_CLEAR_VALUE* pClearValue = nullptr,
                const std::wstring& Name = L"Texture");

    void CreateFromSwapChain(FDevice* pDevice, ID3D12Resource* pResource);
    void Create2D(FDevice* pDevice, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12MA::Allocation* pAllocation, ID3D12Resource* pResource);
    void Destroy();

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return mRTV.GetCpuHandle(); }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return mSRV.GetCpuHandle(); }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return mDSV.GetCpuHandle(); }

private:
    // Automatically freed upon destruction due to RAII wrapper
    FDescriptorAllocation mRTV;
    FDescriptorAllocation mSRV;
    FDescriptorAllocation mDSV;

    UINT mWidth = 0;
    UINT mHeight = 0;
    DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;

    D3D12MA::Allocation* mpAllocation = nullptr;
};
