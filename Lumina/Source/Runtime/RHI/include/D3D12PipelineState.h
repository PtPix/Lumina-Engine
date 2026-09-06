/**
 * @file D3D12PipelineState.h
 * @brief DirectX 12 Graphics Pipeline State Object (PSO) Wrapper and Builder.
 *
 * Encapsulates ID3D12PipelineState. Provides a builder pattern (GraphicsPipelineStateBuilder)
 * to configure and construct PSOs cleanly without manually filling large D3D12 structs.
 */

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

class FD3D12PipelineState
{
public:
    FD3D12PipelineState() = default;
    ~FD3D12PipelineState() { Destroy(); }

    FD3D12PipelineState(const FD3D12PipelineState&) = delete;
    FD3D12PipelineState& operator=(const FD3D12PipelineState&) = delete;

    FD3D12PipelineState(FD3D12PipelineState&&) noexcept = default;
    FD3D12PipelineState& operator=(FD3D12PipelineState&&) noexcept = default;

    void Destroy();

    [[nodiscard]] ID3D12PipelineState* Get() const { return mPipelineState.Get(); }

private:
    friend class FD3D12GraphicsPipelineStateBuilder;
    friend class FD3D12ComputePipelineStateBuilder;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
};

class FD3D12GraphicsPipelineStateBuilder
{
public:
    FD3D12GraphicsPipelineStateBuilder();

    // ------------------------------------------------------------------------
    // Pipeline Configuration Methods
    // ------------------------------------------------------------------------
    FD3D12GraphicsPipelineStateBuilder& SetRootSignature(ID3D12RootSignature* RootSignature);
    FD3D12GraphicsPipelineStateBuilder& SetVertexShader(const void* ByteCode, size_t ByteCodeLength);
    FD3D12GraphicsPipelineStateBuilder& SetPixelShader(const void* ByteCode, size_t ByteCodeLength);
    FD3D12GraphicsPipelineStateBuilder& SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout);
    FD3D12GraphicsPipelineStateBuilder& SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& RtvFormats, DXGI_FORMAT DsvFormat = DXGI_FORMAT_UNKNOWN);
    FD3D12GraphicsPipelineStateBuilder& SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType);

    FD3D12GraphicsPipelineStateBuilder& EnableDepthTest();
    FD3D12GraphicsPipelineStateBuilder& SetDepthStencilFormat(DXGI_FORMAT Format);

    FD3D12GraphicsPipelineStateBuilder& SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
    FD3D12GraphicsPipelineStateBuilder& SetRasterizeState(const D3D12_RASTERIZER_DESC& RasterDesc);
    FD3D12GraphicsPipelineStateBuilder& SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);

    // ------------------------------------------------------------------------
    // Build Output
    // ------------------------------------------------------------------------
    bool Build(ID3D12Device* Device, FD3D12PipelineState& OutPipelineState);

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC mPipelineStateDesc;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputElementDesc;
    std::vector<DXGI_FORMAT> mRtvFormats;
};

class FD3D12ComputePipelineStateBuilder
{
public:
    FD3D12ComputePipelineStateBuilder();

    FD3D12ComputePipelineStateBuilder& SetRootSignature(ID3D12RootSignature* pRootSignature);
    FD3D12ComputePipelineStateBuilder& SetComputeShader(const void* ByteCode, size_t ByteCodeLength);

    bool Build(ID3D12Device* Device, FD3D12PipelineState& OutPipelineState);

private:
    D3D12_COMPUTE_PIPELINE_STATE_DESC mPipelineStateDesc;
};