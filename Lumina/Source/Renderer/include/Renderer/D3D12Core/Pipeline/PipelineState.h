/**
 * @file PipelineState.h
 * @brief DirectX 12 Graphics Pipeline State Object (PSO) Wrapper and Builder.
 *
 * Encapsulates ID3D12PipelineState. Provides a builder pattern (GraphicsPipelineStateBuilder)
 * to configure and construct PSOs cleanly without manually filling large D3D12 structs.
 */

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

class FPipelineState
{
public:
    FPipelineState() = default;
    ~FPipelineState() { Destroy(); }

    FPipelineState(const FPipelineState&) = delete;
    FPipelineState& operator=(const FPipelineState&) = delete;

    FPipelineState(FPipelineState&&) noexcept = default;
    FPipelineState& operator=(FPipelineState&&) noexcept = default;

    void Destroy();

    [[nodiscard]] ID3D12PipelineState* Get() const { return mPipelineState.Get(); }

private:
    friend class FGraphicsPipelineStateBuilder;
    friend class FComputePipelineStateBuilder;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
};

class FGraphicsPipelineStateBuilder
{
public:
    FGraphicsPipelineStateBuilder();

    // ------------------------------------------------------------------------
    // Pipeline Configuration Methods
    // ------------------------------------------------------------------------
    FGraphicsPipelineStateBuilder& SetRootSignature(ID3D12RootSignature* RootSignature);
    FGraphicsPipelineStateBuilder& SetVertexShader(const void* ByteCode, size_t ByteCodeLength);
    FGraphicsPipelineStateBuilder& SetPixelShader(const void* ByteCode, size_t ByteCodeLength);
    FGraphicsPipelineStateBuilder& SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout);
    FGraphicsPipelineStateBuilder& SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& RtvFormats, DXGI_FORMAT DsvFormat = DXGI_FORMAT_UNKNOWN);
    FGraphicsPipelineStateBuilder& SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType);

    FGraphicsPipelineStateBuilder& EnableDepthTest();
    FGraphicsPipelineStateBuilder& SetDepthStencilFormat(DXGI_FORMAT Format);

    FGraphicsPipelineStateBuilder& SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
    FGraphicsPipelineStateBuilder& SetRasterizeState(const D3D12_RASTERIZER_DESC& RasterDesc);
    FGraphicsPipelineStateBuilder& SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);

    // ------------------------------------------------------------------------
    // Build Output
    // ------------------------------------------------------------------------
    bool Build(ID3D12Device* Device, FPipelineState& OutPipelineState);

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC mPipelineStateDesc;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputElementDesc;
    std::vector<DXGI_FORMAT> mRtvFormats;
};

class FComputePipelineStateBuilder
{
public:
    FComputePipelineStateBuilder();

    FComputePipelineStateBuilder& SetRootSignature(ID3D12RootSignature* pRootSignature);
    FComputePipelineStateBuilder& SetComputeShader(const void* ByteCode, size_t ByteCodeLength);

    bool Build(ID3D12Device* Device, FPipelineState& OutPipelineState);

private:
    D3D12_COMPUTE_PIPELINE_STATE_DESC mPipelineStateDesc;
};