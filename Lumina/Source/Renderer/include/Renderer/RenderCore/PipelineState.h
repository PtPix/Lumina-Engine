#pragma once

#include <d3d12.h>
#include <vector>

class PipelineState
{
public:
    PipelineState() = default;
    ~PipelineState();

    void Destroy();
    [[nodiscard]] ID3D12PipelineState* Get() const { return mPipelineState; }

private:
    friend class GraphicsPipelineStateBuilder;
    ID3D12PipelineState* mPipelineState = nullptr;
};

class GraphicsPipelineStateBuilder
{
public:
    GraphicsPipelineStateBuilder();

    GraphicsPipelineStateBuilder& SetRootSignature(ID3D12RootSignature* RootSignature);
    GraphicsPipelineStateBuilder& SetVertexShader(const void* ByteCode, size_t ByteCodeLength);
    GraphicsPipelineStateBuilder& SetPixelShader(const void* ByteCode, size_t ByteCodeLength);
    GraphicsPipelineStateBuilder& SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout);
    GraphicsPipelineStateBuilder& SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& RtvFormats, DXGI_FORMAT DsvFormat = DXGI_FORMAT_UNKNOWN);
    GraphicsPipelineStateBuilder& SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType);

    GraphicsPipelineStateBuilder& EnableDepthTest();
    GraphicsPipelineStateBuilder& SetDepthStencilFormat(DXGI_FORMAT Format);



    GraphicsPipelineStateBuilder& SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
    GraphicsPipelineStateBuilder& SetRasterizeState(const D3D12_RASTERIZER_DESC& RasterDesc);
    GraphicsPipelineStateBuilder& SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);

    bool Build(ID3D12Device* Device, PipelineState& OutPipelineState);

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC mPipelineStateDesc;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputElementDesc;
    std::vector<DXGI_FORMAT> mRtvFormats;
};
