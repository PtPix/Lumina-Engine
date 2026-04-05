#include "Renderer/Rendering/DeferredShadingRenderer.h"

#include "../../../Engine/include/Assets/StaticModel.h"

#include "Renderer/RenderCore/Texture.h"

bool FDeferredShadingRenderer::Initialize(GraphicsDevice* pGraphicsDevice, uint32_t Width, uint32_t Height)
{
    mGraphicsDevice = pGraphicsDevice;
    mWidth = Width;
    mHeight = Height;

    if (!CreateGlobalRootSignature())
    {
        return false;
    }

    mSceneRenderTargets.Initialize(
        mGraphicsDevice->GetDevice().GetDevicePtr(),
        mGraphicsDevice->GetAllocator(),
        &mGraphicsDevice->GetRTVHeap(),
        &mGraphicsDevice->GetCbvSrvUavHeap(),
        &mGraphicsDevice->GetDsvHeap(), mWidth, mHeight);

    if (!InitMaterials())
    {
        return false;
    }



    return true;
}

void FDeferredShadingRenderer::Render(ID3D12GraphicsCommandList* pCommandList, const FSceneView& View,
    const FScene& Scene)
{
    ID3D12DescriptorHeap* ppHeaps[] = { mGraphicsDevice->GetCbvSrvUavHeap().GetHeap() };
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->SetGraphicsRootSignature(mGlobalRootSignature.Get());

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(mWidth), static_cast<float>(mHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = {0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight)};
    pCommandList->RSSetViewports(1, &viewport);
    pCommandList->RSSetScissorRects(1, &scissorRect);

    FViewUniformData ViewUniformData = {};
    ViewUniformData.ViewProjectionMatrix = DirectX::XMMatrixTranspose(View.ViewProjectionMatrix);
    ViewUniformData.InverseViewProjectionMatrix = DirectX::XMMatrixTranspose(View.InverseViewProjectionMatrix);
    ViewUniformData.CameraPosition = DirectX::XMFLOAT4(View.CameraPosition.x, View.CameraPosition.y, View.CameraPosition.z,1.0f);
    ViewUniformData.ScreenResolution = DirectX::XMFLOAT2(View.ViewportWidth, View.ViewportHeight);
    FDynamicAllocation ViewUniformDataAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FViewUniformData));
    memcpy(ViewUniformDataAllocation.CpuAddress, &ViewUniformData, sizeof(FViewUniformData));
    pCommandList->SetGraphicsRootConstantBufferView(0, ViewUniformDataAllocation.GpuAddress);

    mSceneRenderTargets.Clear(pCommandList);
    mSceneRenderTargets.BindBasePass(pCommandList);

    RenderPass(pCommandList, View, Scene, ERenderPass::Skybox);
    RenderPass(pCommandList, View, Scene, ERenderPass::BasePass);

    mSceneRenderTargets.TransitionToLightingPass(pCommandList);

    D3D12_CPU_DESCRIPTOR_HANDLE BackBufferRTV = mGraphicsDevice->GetSwapChain().GetCurrentBackBufferRTVHandle();
    pCommandList->OMSetRenderTargets(1, &BackBufferRTV, false, nullptr);
    const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    pCommandList->ClearRenderTargetView(BackBufferRTV, ClearColor, 0, nullptr);
    RenderDeferredLighting(pCommandList, View, Scene);

    mSceneRenderTargets.TransitionToBasePass(pCommandList);
}

void FDeferredShadingRenderer::Destroy()
{
    mSceneRenderTargets.DestroyRenderTargets();
    mGraphicsDevice = nullptr;
    mDeferredLightingMaterial.Destroy();
}

bool FDeferredShadingRenderer::CreateGlobalRootSignature()
{
    RootSignatureBuilder Builder;

    Builder.AddConstantBufferView(0)
        .AddConstantBufferView(1)
        .AddConstantBufferView(2)
        .AddConstantBufferView(3)
        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 5, 0, D3D12_SHADER_VISIBILITY_PIXEL)
        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL)
        .AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC)
        .AddStaticSampler(1, 0, D3D12_FILTER_MIN_MAG_MIP_POINT)
        .AllowInputLayout();

    if (!Builder.Build(mGraphicsDevice->GetDevice().GetDevicePtr(), mGlobalRootSignature))
    {
        Log::Error("Failed to build Global Root Signature!");
        return false;
    }

    return true;
}

bool FDeferredShadingRenderer::InitMaterials()
{
    if (!mDeferredLightingMaterial.Initialize(mGraphicsDevice->GetDevice().GetDevicePtr(), &mGlobalRootSignature))
    {
        return false;
    }

    return true;
}

void FDeferredShadingRenderer::RenderPass(ID3D12GraphicsCommandList* pCommandList, const FSceneView& View,
    const FScene& Scene, ERenderPass Pass)
{
    pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (auto& Node : Scene.GetRenderNodes())
    {
        if (!Node.pMesh || !Node.pMaterial || !Node.pMaterial->SupportsPass(Pass))
        {
            continue;
        }

        Node.pMaterial->Bind(pCommandList);

        FPrimitiveUniformData ModelData = {};

        if (Pass == ERenderPass::Skybox)
        {
            DirectX::XMMATRIX SkyTranslation = DirectX::XMMatrixTranslation(View.CameraPosition.x, View.CameraPosition.y, View.CameraPosition.z);
            ModelData.ModelMatrix = DirectX::XMMatrixTranspose(Node.ModelMatrix * SkyTranslation);
        }
        else
        {
            ModelData.ModelMatrix = DirectX::XMMatrixTranspose(Node.ModelMatrix);
        }

        FDynamicAllocation ModelAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FPrimitiveUniformData));
        memcpy(ModelAllocation.CpuAddress, &ModelData, sizeof(FPrimitiveUniformData));
        pCommandList->SetGraphicsRootConstantBufferView(2, ModelAllocation.GpuAddress);

        if (Pass == ERenderPass::BasePass)
        {
            FMaterialUniformData MaterialData = {};
            MaterialData.Metallic = Scene.Metallic;
            MaterialData.Roughness = Scene.Roughness;
            MaterialData.AO = Scene.AO;
            FDynamicAllocation MaterialAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FMaterialUniformData));
            memcpy(MaterialAllocation.CpuAddress, &MaterialData, sizeof(FMaterialUniformData));
            pCommandList->SetGraphicsRootConstantBufferView(3, MaterialAllocation.GpuAddress);
        }

        Node.pMesh->Draw(pCommandList);
    }
}

void FDeferredShadingRenderer::RenderDeferredLighting(ID3D12GraphicsCommandList* pCommandList, const FSceneView& View,
    const FScene& Scene)
{
    mDeferredLightingMaterial.Bind(pCommandList);
    pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->SetGraphicsRootDescriptorTable(4, Scene.SkyboxTexture->SourceView.GetGpuDescriptorHandle());
    pCommandList->SetGraphicsRootDescriptorTable(5, mSceneRenderTargets.GetSrvTable().GetGpuDescriptorHandle());

    // Upload light data
    FDirectionalLightUniformData MainLight = {};
    MainLight.LightColor = DirectX::XMFLOAT4(Scene.MainLight.Color.x, Scene.MainLight.Color.y, Scene.MainLight.Color.z, 1.0f);
    MainLight.LightDirection = DirectX::XMFLOAT4(Scene.MainLight.Direction.x, Scene.MainLight.Direction.y, Scene.MainLight.Direction.z, 1.0f);
    FDynamicAllocation MainLightAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FDirectionalLightUniformData));
    memcpy(MainLightAllocation.CpuAddress, &MainLight, sizeof(FDirectionalLightUniformData));
    pCommandList->SetGraphicsRootConstantBufferView(1, MainLightAllocation.GpuAddress);

    pCommandList->DrawInstanced(3, 1, 0 ,0);
}
