#include <d3d12.h>

#include "Editor/TestLayer/PBRModelTestLayer.h"

#include "Assets/StaticModel.h"
#include "Engine/Input.h"
#include "ImGUI/imgui.h"
#include "Logger/Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Pipeline/ShaderCompiler.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/Managers/FTextureManager.h"

void PBRModelTestLayer::OnAttach()
{
    LUMINA_LOG_INFO(App, "PBRModelTestLayer Attaching...");

    // Texture Load
    FResourceUploader* pUploader = Renderer::GetUploader();
    pUploader->BeginUpload();
    uint32_t AlbedoIndex = TextureManager::LoadTexture("Assets/Textures/Radio/T_HandRadio_BaseColor.png", pUploader, true);
    uint32_t NormalIndex = TextureManager::LoadTexture("Assets/Textures/Radio/T_HandRadio_Normal.png", pUploader, false);
    uint32_t ORMIndex = TextureManager::LoadTexture("Assets/Textures/Radio/T_HandRadio_ORM.png", pUploader, false);
    pUploader->EndUpLoadAndExecute();
    pUploader->FlushAndSync();

    // Register a Material
    FPBRMaterial RadioMaterial;
    RadioMaterial.SetAlbedoTexture(AlbedoIndex);
    RadioMaterial.SetNormalTexture(NormalIndex);
    RadioMaterial.SetORMTexture(ORMIndex);

    uint32_t MaterialID = mScene.AddMaterial(RadioMaterial);

    StaticModel Radio;
    Radio.LoadFromFile("Assets/Models/Radio/SM_HandRadio.fbx");
    const auto& RadioMeshes = Radio.GetMeshesData();

    for (int i = 0; i < RadioMeshes.size(); i++)
    {
        FMesh* pGpuMesh = Renderer::CreateMesh(RadioMeshes[i]);
        mLoadedMeshes.push_back(pGpuMesh);

        FGameObject RadioObject;
        RadioObject.pMesh = pGpuMesh;
        RadioObject.Transform = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
        RadioObject.MaterialIndex = MaterialID;

        mScene.AddGameObject(RadioObject);

    }
    FGlobalPassData InitData;
    InitData.SunDirection = { 0.577f, -0.577f, 0.577f };
    InitData.SunColor = { 1.0f, 0.9f, 0.8f, 1.0f };
    InitData.SunIntensity = 3.14f;
    mScene.SetGlobalData(InitData);

    // InitBasePassPipeline();
    mCamera.SetLens(DirectX::XM_PIDIV4, static_cast<float>(1280) / static_cast<float>(720), 0.1f, 1000.0f);

    // 在窗口初始化 / Resize 时调用：
    D3D12_CLEAR_VALUE depthClear = {};
    depthClear.Format = DXGI_FORMAT_D32_FLOAT;
    depthClear.DepthStencil.Depth = 1.0f;
    depthClear.DepthStencil.Stencil = 0;

    mDepthBuffer.Create(
        D3D12Backend::GetDevice(),
        D3D12Backend::GetAllocator(),
        1280, 720,                 // 屏幕宽、高
        DXGI_FORMAT_D32_FLOAT,         // 深度格式
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, // 🚨 核心标志：允许作为 DSV
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClear,
        L"MainDepthBuffer"
    );
}

void PBRModelTestLayer::OnDetach()
{
    D3D12Backend::FlushGPU();

    mDepthBuffer.Destroy();

    for (auto& LoadedMesh : mLoadedMeshes)
    {
        LoadedMesh->Destroy();
    }
}

void PBRModelTestLayer::OnUpdate(double DeltaTime)
{
    if (Input::IsMouseButtonDown(EMouseButton::Right))
    {
        float DeltaX = Input::GetMouseDeltaX();
        float DeltaY = Input::GetMouseDeltaY();
        mCamera.AddRotationInput(DeltaX, DeltaY);
    }

    if (Input::IsKeyDown(EKeyCode::W)) mCamera.AddMovementInput(0.0f, 0.0f, 1.0f);
    if (Input::IsKeyDown(EKeyCode::S)) mCamera.AddMovementInput(0.0f, 0.0f, -1.0f);
    if (Input::IsKeyDown(EKeyCode::D)) mCamera.AddMovementInput(1.0f, 0.0f, 0.0f);
    if (Input::IsKeyDown(EKeyCode::A)) mCamera.AddMovementInput(-1.0f, 0.0f, 0.0f);
    if (Input::IsKeyDown(EKeyCode::E)) mCamera.AddMovementInput(0.0f, 1.0f, 0.0f);
    if (Input::IsKeyDown(EKeyCode::Q)) mCamera.AddMovementInput(0.0f, -1.0f, 0.0f);

    mCamera.Update(DeltaTime);

    DirectX::XMMATRIX viewMat = mCamera.GetViewMatrix();
    DirectX::XMMATRIX projMat = mCamera.GetProjectionMatrix();
    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(viewMat, projMat);

    FGlobalPassData GlobalData = mScene.GetGlobalPassData();
    GlobalData.ViewProjectionMatrix = DirectX::XMMatrixTranspose(viewProj);
    GlobalData.CameraPosition = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    mScene.SetGlobalData(GlobalData);

    mScene.ExtractSceneView(mSceneView);
}

void PBRModelTestLayer::OnRender(FCommandContext* pCommandContext)
{
    D3D12_CPU_DESCRIPTOR_HANDLE BackBufferRTV = D3D12Backend::GetCurrentBackBufferRTV();
    D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = mDepthBuffer.GetDSV();

    pCommandContext->SetRenderTargets(1, &BackBufferRTV, &DsvHandle);

    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    pCommandContext->ClearRenderTargetView(BackBufferRTV, clearColor);
    pCommandContext->ClearDepthStencilView(DsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

    float Width = 1280; float Height = 720;
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, Width, Height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)Width, (LONG)Height };
    pCommandContext->SetViewport(viewport);
    pCommandContext->SetScissorRect(scissorRect);

    Renderer::RenderSceneView(pCommandContext, mSceneView);
}

void PBRModelTestLayer::OnRenderUI()
{
    // ImGui::Begin("PBR Test Layer");

    // ImGui::End();
}

