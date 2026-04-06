#pragma once

#include <string>
#include <d3d12.h>
#include <vector>
#include <cstdint>

#include "../D3D12Core/PipelineState.h"
#include "Renderer/D3D12Core/RootSignature.h"

struct ID3D12Device;
struct ID3D12GraphicsCommandList;

enum class ERenderPass : uint32_t
{
    None = 0,
    BasePass = 1 << 0,
    Skybox = 1 << 1,
    DeferredLighting = 1 << 2,
    Shadow = 1 << 3,
    Transparent = 1 << 4,
};

inline ERenderPass operator|(ERenderPass a, ERenderPass b)
{
    return static_cast<ERenderPass>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline uint32_t operator&(ERenderPass a, ERenderPass b)
{
    return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
}

struct FMaterialInitDesc
{
    std::wstring VertexShaderFilePath;
    std::string VertexShaderEntryPoint = "VSMain";
    std::wstring PixelShaderFilePath;
    std::string PixelShaderEntryPoint = "PSMain";

    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements;
    std::vector<DXGI_FORMAT> RenderTargetViewFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
    DXGI_FORMAT DepthStencilViewFormat = DXGI_FORMAT_D32_FLOAT;
    bool bEnableDepthTest = true;
};

class MaterialBase
{
public:
    MaterialBase() = default;
    virtual ~MaterialBase() = default;

    virtual bool Initialize(ID3D12Device* Device, RootSignature* RootSig) = 0;
    virtual void Bind(ID3D12GraphicsCommandList* CommandList) const = 0;
    virtual void Destroy() = 0;

    [[nodiscard]] ERenderPass GetRenderPassFlags() const { return mRenderPassFlags; }
    [[nodiscard]] bool SupportsPass(ERenderPass Pass) const { return (mRenderPassFlags & Pass) != 0; }

    void SetSrvTable(D3D12_GPU_DESCRIPTOR_HANDLE Table) { mSrvTable = Table; }

protected:
    bool InitializePipeline(ID3D12Device* Device, const FMaterialInitDesc& Desc);

    ERenderPass mRenderPassFlags = ERenderPass::None;
    RootSignature* mRootSignature = nullptr;
    PipelineState mPipelineState;
    D3D12_GPU_DESCRIPTOR_HANDLE mSrvTable = {};
};