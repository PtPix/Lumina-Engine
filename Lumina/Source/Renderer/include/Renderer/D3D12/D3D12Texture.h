/**
 * @file D3D12Texture.h
 * @brief GPU Texture resource wrapper.
 *
 * Support 2D / 2DArray / 3D / Cube / MipChain / UAV.
 * SRV / UAV will be in bindless heap.
 */

#pragma once

#include "D3D12GpuResource.h"
#include "D3D12DescriptorAllocation.h"
#include "D3D12MemAlloc.h"

#include <string>
#include <vector>
#include <cstdint>

class FD3D12Device;

enum class ED3D12TextureDimension : uint8_t
{
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube,
};

enum class ED3D12TextureFlags : uint32_t
{
    None = 0,
    AllowRenderTarget = 1u << 0,
    AllowDepthStencil = 1u << 1,
    AllowUnorderedAccess = 1u << 2,
    DenyShaderResource = 1u << 3,
};

inline ED3D12TextureFlags operator|(ED3D12TextureFlags A, ED3D12TextureFlags B)
{
    return static_cast<ED3D12TextureFlags>(static_cast<uint32_t>(A) | static_cast<uint32_t>(B));
}
inline ED3D12TextureFlags& operator|=(ED3D12TextureFlags& A, ED3D12TextureFlags B) { A = A | B; return A; }
inline bool HasFlag(ED3D12TextureFlags Value, ED3D12TextureFlags Flag)
{
    return (static_cast<uint32_t>(Value) & static_cast<uint32_t>(Flag)) != 0;
}

struct FD3D12TextureDesc
{
    ED3D12TextureDimension Dimension = ED3D12TextureDimension::Texture2D;

    uint32_t Width = 1;
    uint32_t Height = 1;
    uint32_t DepthOrArraySize = 1;

    uint32_t MipLevels = 1;

    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;

    ED3D12TextureFlags Flags = ED3D12TextureFlags::None;

    uint32_t SampleCount = 1;

    bool bUseClearValue = false;
    D3D12_CLEAR_VALUE ClearValue = {};

    D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON;

    std::wstring DebugName = L"Texture";
};

class FD3D12Texture : public D3D12GpuResource
{
public:
    static constexpr  uint32_t InvalidBindlessIndex = UINT32_MAX;

    FD3D12Texture() = default;
    ~FD3D12Texture() override { Destroy(); }

    FD3D12Texture(const FD3D12Texture&) = delete;
    FD3D12Texture& operator=(const FD3D12Texture&) = delete;

    FD3D12Texture(FD3D12Texture&& Other) noexcept;
    FD3D12Texture& operator=(FD3D12Texture&& Other) noexcept;

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    bool Create(FD3D12Device* pDevice, D3D12MA::Allocator* pAllocator, const FD3D12TextureDesc& Desc);

    void CreateFromSwapChain(FD3D12Device* pDevice, ID3D12Resource* pResource);

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
    [[nodiscard]] const FD3D12TextureDesc& GetDesc() const { return mDesc; }
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
    void CreateViews(FD3D12Device* pDevice);
    void CreateSRVInternal(FD3D12Device* pDevice);
    void CreateRTVsInternal(FD3D12Device* pDevice);
    void CreateDSVsInternal(FD3D12Device* pDevice);
    void CreateUAVsInternal(FD3D12Device* pDevice);

    void ReleaseBindlessSlots();

private:
    FD3D12TextureDesc mDesc = {};

    uint32_t mNumMips = 1;
    uint32_t mArraySize = 1;

    DXGI_FORMAT mResourceFormat = DXGI_FORMAT_UNKNOWN;

    bool mbExternalResource = false;

    // RTV, DSV : mip * slices. SRV : 1. UAV: mip
    std::vector<FD3D12DescriptorAllocation> mRTVs;
    std::vector<FD3D12DescriptorAllocation> mDSVs;
    FD3D12DescriptorAllocation mSRV;
    std::vector<FD3D12DescriptorAllocation> mUAVs;

    uint32_t mBindlessSRVIndex = InvalidBindlessIndex;
    std::vector<uint32_t> mBindlessUAVIndices;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> mpAllocation;

    FD3D12Device* mpDevice = nullptr;
};
