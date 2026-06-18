#include <d3d12.h>

#include "Editor/TestLayer/PBRModelTestLayer.h"

#include "Assets/StaticModel.h"
#include "Engine/Input.h"
#include "ImGUI/imgui.h"
#include "Logger/Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Pipeline/ShaderCompiler.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/Managers/TextureManager.h"

void PBRModelTestLayer::OnAttach()
{
    LUMINA_LOG_INFO(App, "PBRModelTestLayer Attaching...");

    // Texture Load
    uint32_t AlbedoIndex = TextureManager::LoadTexture("Assets/Textures/Radio/T_HandRadio_BaseColor.png", true);
    uint32_t NormalIndex = TextureManager::LoadTexture("Assets/Textures/Radio/T_HandRadio_Normal.png", false);
    uint32_t ORMIndex = TextureManager::LoadTexture("Assets/Textures/Radio/T_HandRadio_ORM.png", false);

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

    mCamera.SetLens(DirectX::XM_PIDIV4, static_cast<float>(1280) / static_cast<float>(720), 0.1f, 1000.0f);
}

void PBRModelTestLayer::OnDetach()
{
    Renderer::GetD3D12Backend()->FlushAllQueues();

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

void PBRModelTestLayer::OnRender(FRenderGraph& RenderGraph)
{
    FRGTextureDesc DepthDesc = {};
    DepthDesc.Width = Renderer::GetD3D12Backend()->GetWidth();
    DepthDesc.Height = Renderer::GetD3D12Backend()->GetHeight();
    DepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.Usage = ERGTextureUsage::DepthStencil;
    DepthDesc.bUseClearValue = true;
    DepthDesc.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.ClearValue.DepthStencil.Depth = 1.0f;
    DepthDesc.ClearValue.DepthStencil.Stencil = 0;

    FRGTextureHandle Depth = RenderGraph.CreateTexture("SceneDepth", DepthDesc);
    FRGTextureHandle SceneColor = RenderGraph.GetTexture("EditorViewport.SceneColor");

    RenderGraph.AddPass("BasePass")
        .WriteRenderTarget(SceneColor, ERGLoadOp::Clear)
        .WriteDepth(Depth, ERGLoadOp::Clear)
        .Execute([this](FRenderGraphContext& Context)
        {
            FCommandContext* Cmd = Context.GetCommandContext();

            float Width = static_cast<float>(Renderer::GetD3D12Backend()->GetWidth());
            float Height = static_cast<float>(Renderer::GetD3D12Backend()->GetHeight());

            D3D12_VIEWPORT Viewport = { 0.0f, 0.0f, Width, Height, 0.0f, 1.0f };
            D3D12_RECT Scissor = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };

            Cmd->SetViewport(Viewport);
            Cmd->SetScissorRect(Scissor);

            Renderer::RenderSceneView(Cmd, mSceneView);
        });
}

void PBRModelTestLayer::OnRenderUI()
{
}

