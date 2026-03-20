#pragma once

#include <string>
#include <d3d12.h>
#include <vector>

#include "PipelineState.h"
#include "Renderer/RenderCore/RootSignature.h"

struct ID3D12Device;
struct ID3D12GraphicsCommandList;

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

class Material
{
public:
    Material() = default;
    ~Material() = default;

    bool Initialize(ID3D12Device* Device, const FMaterialInitDesc& MaterialDesc);

    void Bind(ID3D12GraphicsCommandList* CommandList) const;

    [[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return mRootSignature->Get(); }
    void SetRootSignature(RootSignature* InRootSignature) { mRootSignature = InRootSignature; }
    [[nodiscard]] ID3D12PipelineState* GetPipelineState() const { return mPipelineState.Get(); }

    void Destroy();
private:
    RootSignature* mRootSignature;
    PipelineState mPipelineState;
};