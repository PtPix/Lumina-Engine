#pragma once

#include <cstdint>
#include <d3d12.h>
#include <functional>

#include "../D3D12/D3D12Texture.h"

class FD3D12CommandContext;
class FD3D12GpuResource;
class FD3D12Device;

enum class ERGTextureUsage : uint16_t
{
    None = 0,
    RenderTarget = 1 << 0,
    DepthStencil = 1 << 1,
    ShaderResource = 1 << 2,
    UnorderedAccess = 1 << 3,
    Present = 1 << 4,
};

inline ERGTextureUsage operator|(ERGTextureUsage A, ERGTextureUsage B)
{
    return static_cast<ERGTextureUsage>(static_cast<uint16_t>(A) | static_cast<uint16_t>(B));
}

inline bool HasUsage(ERGTextureUsage Usage, ERGTextureUsage Flag)
{
    return (static_cast<uint16_t>(Usage) & static_cast<uint16_t>(Flag)) != 0;
}

enum class ERGLoadOp
{
    Load, Clear
};

struct FRGTextureDesc
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    ERGTextureUsage Usage = ERGTextureUsage::None;

    bool bUseClearValue = false;
    D3D12_CLEAR_VALUE ClearValue = {};
};

struct FRGTextureHandle
{
    static constexpr uint32_t InvalidIndex = UINT32_MAX;
    uint32_t Index = InvalidIndex;
    [[nodiscard]] bool IsValid() const { return Index != InvalidIndex; }
};

class FRenderGraph;

class FRenderGraphContext
{
public:
    [[nodiscard]] FD3D12CommandContext* GetCommandContext() const { return mpCommandContext; }

    [[nodiscard]] FD3D12GpuResource* GetResource(FRGTextureHandle Handle) const;
    [[nodiscard]] FD3D12Texture* GetTexture(FRGTextureHandle Handle) const;

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(FRGTextureHandle Handle) const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(FRGTextureHandle Handle) const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSRV(FRGTextureHandle Handle) const;

    [[nodiscard]] uint32_t GetSRVIndex(FRGTextureHandle Handle) const;
    [[nodiscard]] uint32_t GetUAVIndex(FRGTextureHandle Handle, uint32_t Mip = 0) const;

private:
    friend class FRenderGraph;

    FRenderGraph* mpGraph = nullptr;
    FD3D12CommandContext* mpCommandContext = nullptr;
};

class FRenderGraphPassBuilder
{
public:
    FRenderGraphPassBuilder& ReadTexture(FRGTextureHandle Texture);
    FRenderGraphPassBuilder& WriteRenderTarget(FRGTextureHandle Texture, ERGLoadOp LoadOp = ERGLoadOp::Load);
    FRenderGraphPassBuilder& WriteDepth(FRGTextureHandle Texture, ERGLoadOp LoadOp = ERGLoadOp::Load);
    FRenderGraphPassBuilder& ReadWriteUAV(FRGTextureHandle Texture);

    FRenderGraphPassBuilder& Execute(std::function<void(FRenderGraphContext&)> ExecuteFunc);

private:
    friend class FRenderGraph;

    FRenderGraphPassBuilder(FRenderGraph* pGraph, uint32_t PassIndex) : mpGraph(pGraph), mPassIndex(PassIndex) {}

    FRenderGraph* mpGraph = nullptr;
    uint32_t mPassIndex = 0;
};

class FRenderGraph
{
public:
    void Initialize(FD3D12Device* pDevice, D3D12MA::Allocator* pAllocator);
    void Shutdown();

    void Reset();
    void ClearImportedResources();

    FRGTextureHandle CreateTexture(const char* Name, const FRGTextureDesc& Desc);
    FRGTextureHandle GetTexture(const char* Name);

    FRGTextureHandle ImportBackBuffer(const char* Name, FD3D12GpuResource* pResource, D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
        uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12_RESOURCE_STATES InitialState);

    FRenderGraphPassBuilder AddPass(const char* Name);

    void Compile();
    void Execute(FD3D12CommandContext* pCommandContext);

private:
    friend class FRenderGraphContext;
    friend class FRenderGraphPassBuilder;

    struct FResource
    {
        std::string Name;
        FRGTextureDesc Desc;

        bool bImported = false;
        FD3D12GpuResource* pImportedResource = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE ImportedRTV = {};

        FD3D12Texture Texture;
    };

    struct FTextureAccess
    {
        FRGTextureHandle Texture;
        D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;
    };

    struct FRenderTargetAccess
    {
        FRGTextureHandle Texture;
        ERGLoadOp LoadOp = ERGLoadOp::Load;
    };

    struct FPass
    {
        std::string Name;

        std::vector<FTextureAccess> Reads;
        std::vector<FTextureAccess> UAVs;
        std::vector<FRenderTargetAccess> RenderTargets;

        bool bHasDepth = false;
        FRenderTargetAccess Depth;

        std::function<void(FRenderGraphContext&)> ExecuteFunc;
    };

private:
    FResource& GetResourceInternal(FRGTextureHandle Handle);
    const FResource& GetResourceInternal(FRGTextureHandle Handle) const;

    [[nodiscard]] FD3D12GpuResource* GetGpuResource(FRGTextureHandle Handle) const;

    bool CreatePhysicalTexture(FResource& Resource);
    bool IsSameDesc(const FRGTextureDesc& A, const FRGTextureDesc& B) const;

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTVInternal(FRGTextureHandle Handle) const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSVInternal(FRGTextureHandle Handle) const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSRVInternal(FRGTextureHandle Handle) const;

    uint32_t GetSRVIndexInternal(FRGTextureHandle Handle) const;
    uint32_t GetUAVIndexInternal(FRGTextureHandle Handle, uint32_t Mip) const;

private:
    FD3D12Device* mpDevice = nullptr;
    D3D12MA::Allocator* mpAllocator = nullptr;

    std::vector<FResource> mResources;
    std::unordered_map<std::string, uint32_t> mResourceNameToIndex;

    std::vector<FPass> mPasses;
};