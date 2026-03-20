#include "Renderer/Rendering/DeferredShadingRenderer.h"

#include "../../../Engine/include/Assets/StaticModel.h"
#include "Logger/Logger.h"
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
    // Bind Descriptor Heaps and set Rootsignature
    ID3D12DescriptorHeap* ppHeaps[] = { mGraphicsDevice->GetCbvSrvUavHeap().GetHeap() };
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->SetGraphicsRootSignature(mGlobalRootSignature.Get());

    // Set viewport and scissorRect
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(mWidth), static_cast<float>(mHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = {0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight)};
    pCommandList->RSSetViewports(1, &viewport);
    pCommandList->RSSetScissorRects(1, &scissorRect);

    // Upload Per Frame Data
    FViewUniformData ViewUniformData = {};
    ViewUniformData.ViewProjectionMatrix = DirectX::XMMatrixTranspose(View.ViewProjectionMatrix);
    ViewUniformData.InverseViewProjectionMatrix = DirectX::XMMatrixTranspose(View.InverseViewProjectionMatrix);
    ViewUniformData.CameraPosition = DirectX::XMFLOAT4(View.CameraPosition.x, View.CameraPosition.y, View.CameraPosition.z,1.0f);
    ViewUniformData.ScreenResolution = DirectX::XMFLOAT2(View.ViewportWidth, View.ViewportHeight);
    FDynamicAllocation ViewUniformDataAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FViewUniformData));
    memcpy(ViewUniformDataAllocation.CpuAddress, &ViewUniformData, sizeof(FViewUniformData));
    pCommandList->SetGraphicsRootConstantBufferView(0, ViewUniformDataAllocation.GpuAddress);

    // Base Pass
    mSceneRenderTargets.Clear(pCommandList);
    mSceneRenderTargets.BindBasePass(pCommandList);

    RenderSkybox(pCommandList, View, Scene);
    RenderBasePass(pCommandList, View, Scene);

    // Transition to lighting pass
    mSceneRenderTargets.TransitionToLightingPass(pCommandList);

    // Lighting pass
    D3D12_CPU_DESCRIPTOR_HANDLE BackBufferRTV = mGraphicsDevice->GetSwapChain().GetCurrentBackBufferRTVHandle();
    pCommandList->OMSetRenderTargets(1, &BackBufferRTV, false, nullptr);
    const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 纯黑底色
    pCommandList->ClearRenderTargetView(BackBufferRTV, ClearColor, 0, nullptr);
    RenderDeferredLighting(pCommandList, View, Scene);

    // Transition back
    mSceneRenderTargets.TransitionToBasePass(pCommandList);
}

void FDeferredShadingRenderer::Destroy()
{
    mSceneRenderTargets.DestroyRenderTargets();
    mGraphicsDevice = nullptr;
    mBasePassMaterial.Destroy();
    mSkyboxMaterial.Destroy();
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
    // Init BasePass Material
    mBasePassMaterial.SetRootSignature(&mGlobalRootSignature);
    FMaterialInitDesc MaterialInitDesc = {};
    MaterialInitDesc.VertexShaderFilePath = L"Shaders/BasePass.hlsl";
    MaterialInitDesc.PixelShaderFilePath = L"Shaders/BasePass.hlsl";
    MaterialInitDesc.InputElements = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    MaterialInitDesc.RenderTargetViewFormats = {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM
    };
    MaterialInitDesc.DepthStencilViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (!mBasePassMaterial.Initialize(mGraphicsDevice->GetDevice().GetDevicePtr(), MaterialInitDesc))
    {
        return false;
    }

    // Init Skybox Material
    mSkyboxMaterial.SetRootSignature(&mGlobalRootSignature);
    FMaterialInitDesc SkyboxDesc = {};
    SkyboxDesc.VertexShaderFilePath = L"Shaders/SkyBox.hlsl";
    SkyboxDesc.PixelShaderFilePath = L"Shaders/SkyBox.hlsl";
    SkyboxDesc.InputElements = MaterialInitDesc.InputElements;
    SkyboxDesc.bEnableDepthTest = false;
    SkyboxDesc.RenderTargetViewFormats = MaterialInitDesc.RenderTargetViewFormats;
    SkyboxDesc.DepthStencilViewFormat = MaterialInitDesc.DepthStencilViewFormat;
    if (!mSkyboxMaterial.Initialize(mGraphicsDevice->GetDevice().GetDevicePtr(), SkyboxDesc))
    {
        return false;
    }

    // Init DeferredShading Material
    mDeferredLightingMaterial.SetRootSignature(&mGlobalRootSignature);
    FMaterialInitDesc LightingDesc = {};
    LightingDesc.VertexShaderFilePath = L"Shaders/DeferredLighting.hlsl";
    LightingDesc.PixelShaderFilePath = L"Shaders/DeferredLighting.hlsl";
    LightingDesc.InputElements.clear();
    LightingDesc.bEnableDepthTest = false;
    if (!mDeferredLightingMaterial.Initialize(mGraphicsDevice->GetDevice().GetDevicePtr(), LightingDesc))
    {
        return false;
    }

    return true;
}

void FDeferredShadingRenderer::RenderBasePass(ID3D12GraphicsCommandList* pCommandList, const FSceneView& View,
    const FScene& Scene)
{
    if (!Scene.CharacterModel)
    {
        return;
    }

    mBasePassMaterial.Bind(pCommandList);
    pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->SetGraphicsRootDescriptorTable(5, Scene.HelmetPBRSrvTable);

    FPrimitiveUniformData ModelData = {};
    ModelData.ModelMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
    FDynamicAllocation ModelAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FPrimitiveUniformData));
    memcpy(ModelAllocation.CpuAddress, &ModelData, sizeof(FPrimitiveUniformData));
    pCommandList->SetGraphicsRootConstantBufferView(2, ModelAllocation.GpuAddress);

    FMaterialUniformData MaterialData = {};
    MaterialData.Metallic = Scene.Metallic;
    MaterialData.Roughness = Scene.Roughness;
    MaterialData.AO = Scene.AO;
    FDynamicAllocation MaterialAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FMaterialUniformData));
    memcpy(MaterialAllocation.CpuAddress, &MaterialData, sizeof(FMaterialUniformData));
    pCommandList->SetGraphicsRootConstantBufferView(3, MaterialAllocation.GpuAddress);

    Scene.CharacterModel->Draw(pCommandList);
}

void FDeferredShadingRenderer::RenderSkybox(ID3D12GraphicsCommandList* pCommandList, const FSceneView& View,
    const FScene& Scene)
{
    if (!Scene.SkyboxModel)
    {
        return;
    }

    mSkyboxMaterial.Bind(pCommandList);
    pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->SetGraphicsRootDescriptorTable(4, Scene.SkyboxTexture->SourceView.GetGpuDescriptorHandle());

    DirectX::XMMATRIX SkyModelMatrix = DirectX::XMMatrixScaling(500.0f, 500.0f, 500.0f);
    DirectX::XMMATRIX SkyTranslation = DirectX::XMMatrixTranslation(View.CameraPosition.x, View.CameraPosition.y, View.CameraPosition.z);
    FPrimitiveUniformData SkyPrimitiveUniformData = {};
    SkyPrimitiveUniformData.ModelMatrix = DirectX::XMMatrixTranspose(SkyModelMatrix * SkyTranslation);
    FDynamicAllocation SkyAllocation = mGraphicsDevice->GetDynamicUploadHeap().Allocate(sizeof(FPrimitiveUniformData));
    memcpy(SkyAllocation.CpuAddress, &SkyPrimitiveUniformData, sizeof(FPrimitiveUniformData));
    pCommandList->SetGraphicsRootConstantBufferView(2, SkyAllocation.GpuAddress);

    Scene.SkyboxModel->Draw(pCommandList);
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
