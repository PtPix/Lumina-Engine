#include "Renderer/RenderCore/DeferredLightingMaterial.h"

bool DeferredLightingMaterial::Initialize(ID3D12Device* Device, RootSignature* RootSig)
{
    mRootSignature = RootSig;

    FMaterialInitDesc Desc;
    Desc.VertexShaderFilePath = L"Shaders/DeferredLighting.hlsl";
    Desc.PixelShaderFilePath = L"Shaders/DeferredLighting.hlsl";
    Desc.InputElements.clear();
    Desc.bEnableDepthTest = false;

    return InitializePipeline(Device, Desc);
}

void DeferredLightingMaterial::Bind(ID3D12GraphicsCommandList* CommandList) const
{
    if (mPipelineState.Get())
    {
        CommandList->SetPipelineState(mPipelineState.Get());
    }
}

void DeferredLightingMaterial::Destroy()
{
    mPipelineState.Destroy();
}
