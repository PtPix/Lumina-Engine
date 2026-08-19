/**
 * @file Texture.h
 * @brief GPU Texture resource wrapper.
 *
 * Support 2D / 2DArray / 3D / Cube / MipChain / UAV.
 * SRV / UAV will be in bindless heap.
 */

#pragma once

#include "Renderer/D3D12Core/Resource/GpuResource.h"
#include "Renderer/D3D12Core/Descriptors/DescriptorAllocation.h"
#include "D3D12MemAlloc.h"

#include <string>
#include <vector>
#include <cstdint>

class FDevice;

enum class ETextureDimension : uint8_t
{
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube,
};

enum class ETextureFlags : uint32_t
{
    None = 0,
    AllowRenderTarget = 1u << 0,
    AllowDepthStencil = 1u << 1,
    AllowUnorderedAccess = 1u << 2,
    DenyShaderResource = 1u << 3,
};

inline ETextureFlags operator|(ETextureFlags A, ETextureFlags B)
{
    return static_cast<ETextureFlags>(static_cast<uint32_t>(A) | static_cast<uint32_t>(B));
}
inline ETextureFlags& operator|=(ETextureFlags& A, ETextureFlags B) { A = A | B; return A; }
inline bool HasFlag(ETextureFlags Value, ETextureFlags Flag)
{
    return (static_cast<uint32_t>(Value) & static_cast<uint32_t>(Flag)) != 0;
}

struct FTextureDesc
{
    ETextureDimension Dimension = ETextureDimension::Texture2D;

    uint32_t Width = 1;
    uint32_t Height = 1;
    uint32_t DepthOrArraySize = 1;

    uint32_t MipLevels = 1;

    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;

    ETextureFlags Flags = ETextureFlags::None;

    uint32_t SampleCount = 1;

    bool bUseClearValue = false;
    D3D12_CLEAR_VALUE ClearValue = {};

    D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON;

    std::wstring DebugName = L"Texture";
};

class FTexture : public GpuResource
{
public:
    static constexpr  uint32_t InvalidBindlessIndex = UINT32_MAX;

    FTexture() = default;
    ~FTexture() override { Destroy(); }

    FTexture(const FTexture&) = delete;
    FTexture& operator=(const FTexture&) = delete;

    FTexture(FTexture&& Other) noexcept;
    FTexture& operator=(FTexture&& Other) noexcept;

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    bool Create(FDevice* pDevice, D3D12MA::Allocator* pAllocator, const FTextureDesc& Desc);

    void CreateFromSwapChain(FDevice* pDevice, ID3D12Resource* pResource);

    void Destroy();

    [[nodiscard]] bool IsValid() const { return mpResource != nullptr; }

    // ------------------------------------------------------------------------
    // Descriptors (Cpu Handles)
    // ------------------------------------------------------------------------

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(uint32_t Mip = 0, uint32_t Slice = 0) const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(uint32_t Mip = 0, uint32_t Slice = 0) const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return mSRV.IsValid() ? mSRV.GetCpuHandle() : D3D12_CPU_DESCRIPTOR_HANDLE{}; }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(uint32_t Mip = 0) const;

    // ------------------------------------------------------------------------
    // Bindless indices
    // ------------------------------------------------------------------------
    [[nodiscard]] uint32_t GetBindlessSRVIndex() const { return mBindlessSRVIndex; }
    [[nodiscard]] uint32_t GetBindlessUAVIndex(uint32_t Mip = 0) const
    {
        return (Mip < mBindlessUAVIndices.size()) ? mBindlessUAVIndices[Mip] : InvalidBindlessIndex;
    }

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    [[nodiscard]] const FTextureDesc& GetDesc() const { return mDesc; }
    [[nodiscard]] uint32_t GetWidth() const { return mDesc.Width; }
    [[nodiscard]] uint32_t GetHeight() const { return mDesc.Height; }
    [[nodiscard]] uint32_t GetMipLevels() const { return mNumMips; }
    [[nodiscard]] uint32_t GetArraySize() const { return mArraySize; }
    [[nodiscard]] DXGI_FORMAT GetFormat() const { return mDesc.Format; }

    [[nodiscard]] uint32_t CalcSubresource(uint32_t Mip, uint32_t Slice = 0) const
    {
        return Mip + Slice * mNumMips;
    }

private:
    void CreateViews(FDevice* pDevice);
    void CreateSRVInternal(FDevice* pDevice);
    void CreateRTVsInternal(FDevice* pDevice);
    void CreateDSVsInternal(FDevice* pDevice);
    void CreateUAVsInternal(FDevice* pDevice);

    void ReleaseBindlessSlots();

private:
    FTextureDesc mDesc = {};

    uint32_t mNumMips = 1;
    uint32_t mArraySize = 1;

    DXGI_FORMAT mResourceFormat = DXGI_FORMAT_UNKNOWN;

    bool mbExternalResource = false;

    // RTV, DSV : mip * slices. SRV : 1. UAV: mip
    std::vector<FDescriptorAllocation> mRTVs;
    std::vector<FDescriptorAllocation> mDSVs;
    FDescriptorAllocation mSRV;
    std::vector<FDescriptorAllocation> mUAVs;

    uint32_t mBindlessSRVIndex = InvalidBindlessIndex;
    std::vector<uint32_t> mBindlessUAVIndices;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> mpAllocation;

    FDevice* mpDevice = nullptr;
};
