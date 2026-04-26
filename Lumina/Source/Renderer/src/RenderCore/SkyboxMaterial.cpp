#include "Renderer/RenderCore/SkyboxMaterial.h"

#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

bool SkyboxMaterial::Initialize(FDevice* Device, RootSignature* RootSig)
{
    mRootSignature = RootSig;

    FMaterialInitDesc Desc;
    Desc.VertexShaderFilePath = L"Shaders/SkyBox.hlsl";
    Desc.PixelShaderFilePath = L"Shaders/SkyBox.hlsl";
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
    Desc.bEnableDepthTest = false;

    return InitializePipeline(Device->GetDevice(), Desc);
}

void SkyboxMaterial::Bind(FCommandContext* Context) const
{
    if (mPipelineState.Get())
    {
        Context->SetPipelineState(mPipelineState.Get());
    }

    BindTextures(*Context);
    // if (mSrvTable.ptr != 0)
    // {
    //     Context->SetGraphicsRootDescriptorTable(4, mSrvTable);
    // }
}

void SkyboxMaterial::Destroy()
{
    mPipelineState.Destroy();
}
