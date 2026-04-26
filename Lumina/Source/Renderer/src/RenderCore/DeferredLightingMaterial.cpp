#include "Renderer/RenderCore/DeferredLightingMaterial.h"

#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

bool DeferredLightingMaterial::Initialize(FDevice* Device, RootSignature* RootSig)
{
    mRootSignature = RootSig;

    FMaterialInitDesc Desc;
    Desc.VertexShaderFilePath = L"Shaders/DeferredLighting.hlsl";
    Desc.PixelShaderFilePath = L"Shaders/DeferredLighting.hlsl";
    Desc.InputElements.clear();
    Desc.bEnableDepthTest = false;

    return InitializePipeline(Device->GetDevice(), Desc);
}

void DeferredLightingMaterial::Bind(FCommandContext* Context) const
{
    if (mPipelineState.Get())
    {
        Context->SetPipelineState(mPipelineState.Get());
    }
}

void DeferredLightingMaterial::Destroy()
{
    mPipelineState.Destroy();
}
