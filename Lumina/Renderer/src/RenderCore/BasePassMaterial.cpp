#include "Renderer/RenderCore/BasePassMaterial.h"

#include "Logger/Logger.h"

bool BasePassMaterial::Initialize(ID3D12Device* Device, RootSignature* RootSig)
{
    mRootSignature = RootSig;

    FMaterialInitDesc Desc;
    Desc.VertexShaderFilePath = L"Shaders/BasePass.hlsl";
    Desc.PixelShaderFilePath = L"Shaders/BasePass.hlsl";
    Desc.InputElements = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    Desc.RenderTargetViewFormats = {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM
    };
    Desc.DepthStencilViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    return InitializePipeline(Device, Desc);
}

void BasePassMaterial::Bind(ID3D12GraphicsCommandList* CommandList) const
{
    if (mPipelineState.Get())
    {
        CommandList->SetPipelineState(mPipelineState.Get());
    }

    if (mSrvTable.ptr != 0)
    {
        CommandList->SetGraphicsRootDescriptorTable(5, mSrvTable);
    }
}

void BasePassMaterial::Destroy()
{
    mPipelineState.Destroy();
}
