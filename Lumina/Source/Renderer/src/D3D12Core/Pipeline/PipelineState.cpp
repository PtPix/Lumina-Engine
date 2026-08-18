#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/D3D12Core/Common.h"

void FPipelineState::Destroy()
{
    mPipelineState.Reset();
}

FGraphicsPipelineStateBuilder::FGraphicsPipelineStateBuilder()
{
    ZeroMemory(&mPipelineStateDesc, sizeof(mPipelineStateDesc));

    // Default Rasterizer State
    mPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    mPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    mPipelineStateDesc.RasterizerState.FrontCounterClockwise = false;
    mPipelineStateDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    mPipelineStateDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    mPipelineStateDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    mPipelineStateDesc.RasterizerState.DepthClipEnable = true;
    mPipelineStateDesc.RasterizerState.MultisampleEnable = false;
    mPipelineStateDesc.RasterizerState.AntialiasedLineEnable = false;
    mPipelineStateDesc.RasterizerState.ForcedSampleCount = 0;
    mPipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Default Blend State
    mPipelineStateDesc.BlendState.AlphaToCoverageEnable = false;
    mPipelineStateDesc.BlendState.IndependentBlendEnable = false;

    D3D12_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc = {
        FALSE, FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    {
        mPipelineStateDesc.BlendState.RenderTarget[i] = DefaultRenderTargetBlendDesc;
    }

    // Default Depth Stencil State
    mPipelineStateDesc.DepthStencilState.DepthEnable = false;
    mPipelineStateDesc.DepthStencilState.StencilEnable = false;

    // Default Pipeline Overrides
    mPipelineStateDesc.SampleMask = UINT_MAX;
    mPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    mPipelineStateDesc.SampleDesc.Count = 1;
    mPipelineStateDesc.SampleDesc.Quality = 0;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetRootSignature(ID3D12RootSignature* RootSignature)
{
    mPipelineStateDesc.pRootSignature = RootSignature;
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetVertexShader(const void* ByteCode, size_t ByteCodeLength)
{
    mPipelineStateDesc.VS = { reinterpret_cast<const BYTE*>(ByteCode), ByteCodeLength };
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetPixelShader(const void* ByteCode, size_t ByteCodeLength)
{
    mPipelineStateDesc.PS = { reinterpret_cast<const BYTE*>(ByteCode), ByteCodeLength };
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetInputLayout(
    const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout)
{
    mInputElementDesc = InputLayout;

    mPipelineStateDesc.InputLayout.pInputElementDescs = mInputElementDesc.data();
    mPipelineStateDesc.InputLayout.NumElements = static_cast<UINT>(InputLayout.size());

    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetRenderTargetFormats(
    const std::vector<DXGI_FORMAT>& RtvFormats, DXGI_FORMAT DsvFormat)
{
    mRtvFormats = RtvFormats;
    mPipelineStateDesc.NumRenderTargets = static_cast<UINT>(RtvFormats.size());

    for (size_t i = 0; i < mRtvFormats.size(); ++i)
    {
        mPipelineStateDesc.RTVFormats[i] = mRtvFormats[i];
    }

    mPipelineStateDesc.DSVFormat = DsvFormat;
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetPrimitiveTopologyType(
    D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
{
    mPipelineStateDesc.PrimitiveTopologyType = TopologyType;
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::EnableDepthTest()
{
    mPipelineStateDesc.DepthStencilState.DepthEnable = true;
    mPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    mPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    mPipelineStateDesc.DepthStencilState.StencilEnable = false;

    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetDepthStencilFormat(DXGI_FORMAT Format)
{
    mPipelineStateDesc.DSVFormat = Format;
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
{
    mPipelineStateDesc.BlendState = BlendDesc;
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetRasterizeState(const D3D12_RASTERIZER_DESC& RasterDesc)
{
    mPipelineStateDesc.RasterizerState = RasterDesc;
    return *this;
}

FGraphicsPipelineStateBuilder& FGraphicsPipelineStateBuilder::SetDepthStencilState(
    const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
{
    mPipelineStateDesc.DepthStencilState = DepthStencilDesc;
    return *this;
}

bool FGraphicsPipelineStateBuilder::Build(ID3D12Device* Device, FPipelineState& OutPipelineState)
{
    if (!mPipelineStateDesc.pRootSignature)
    {
        LUMINA_LOG_ERROR(RHI, "Pipeline State requires a Root Signature!");
        return false;
    }

    OutPipelineState.Destroy();
    HRESULT HResult = Device->CreateGraphicsPipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&OutPipelineState.mPipelineState));

    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Failed to create graphics pipeline state");
        return false;
    }

    return true;
}

FComputePipelineStateBuilder::FComputePipelineStateBuilder()
{
    ZeroMemory(&mPipelineStateDesc, sizeof(mPipelineStateDesc));
    mPipelineStateDesc.NodeMask = 0;
    mPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
}

FComputePipelineStateBuilder & FComputePipelineStateBuilder::SetRootSignature(ID3D12RootSignature *pRootSignature)
{
    mPipelineStateDesc.pRootSignature = pRootSignature;
    return *this;

}

FComputePipelineStateBuilder & FComputePipelineStateBuilder::SetComputeShader(const void *ByteCode,
    size_t ByteCodeLength)
{
    mPipelineStateDesc.CS = { reinterpret_cast<const BYTE*>(ByteCode), ByteCodeLength };
    return *this;
}

bool FComputePipelineStateBuilder::Build(ID3D12Device *Device, FPipelineState &OutPipelineState)
{
    if (!mPipelineStateDesc.pRootSignature)
    {
        LUMINA_LOG_ERROR(RHI, "Compute Pipeline State requires a Root Signature!");
        return false;
    }

    OutPipelineState.Destroy();
    HRESULT HResult = Device->CreateComputePipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&OutPipelineState.mPipelineState));

    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Failed to create compute pipeline state");
        return false;
    }

    return true;
}
