#include "Scene/SceneRenderer.h"
#include "Scene/Scene.h"
#include "Camera/Camera.h"
#include "Scene/ViewInfo.h"
#include "Scene/MeshDrawCommand.h"
#include "FMesh.h"
#include "FMaterial.h"
#include "Passes/BasePass/BasePass.h"
#include "RenderCore.h"

void FSceneRenderer::RenderScene(FScene* Scene, Camera* Camera, FRenderGraph& RenderGraph)
{
    if (!Scene || !Camera)
    {
        return;
    }

    // Step 1: Setup view from camera
    SetupViewFromCamera(Camera);

    // Step 2: Gather draw commands from scene
    GatherDrawCommands(Scene);

    // Step 3: Build and add render passes
    BuildRenderPasses(RenderGraph);
}

void FSceneRenderer::SetupViewFromCamera(Camera* Camera)
{
    // Setup view info from camera
    mViewInfo.SetupFromCamera(Camera, FRendererCore::GetRenderWidth(), FRendererCore::GetRenderHeight());
}

void FSceneRenderer::GatherDrawCommands(FScene* Scene)
{
    mDrawCommands.clear();

    // Get all game objects from scene
    auto& GameObjects = Scene->GetGameObjects();

    for (const auto& GameObject : GameObjects)
    {
        if (!GameObject.pMesh)
        {
            continue;
        }

        FMeshDrawCommand DrawCmd;
        DrawCmd.Mesh = GameObject.pMesh;
        DrawCmd.MaterialIndex = GameObject.MaterialIndex;

        // Store world transform
        DirectX::XMStoreFloat4x4(&DrawCmd.WorldMatrix, DirectX::XMMatrixTranspose(GameObject.Transform));

        mDrawCommands.push_back(DrawCmd);
    }
}

void FSceneRenderer::BuildRenderPasses(FRenderGraph& RenderGraph)
{
    // Create scene color render target
    FRGTextureDesc ColorDesc = {};
    ColorDesc.Width = mViewInfo.RenderTargetWidth;
    ColorDesc.Height = mViewInfo.RenderTargetHeight;
    ColorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ColorDesc.Usage = ERGTextureUsage::RenderTarget | ERGTextureUsage::ShaderResource;
    ColorDesc.bUseClearValue = true;
    ColorDesc.ClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ColorDesc.ClearValue.Color[0] = 0.1f;
    ColorDesc.ClearValue.Color[1] = 0.1f;
    ColorDesc.ClearValue.Color[2] = 0.1f;
    ColorDesc.ClearValue.Color[3] = 1.0f;

    FRGTextureHandle SceneColor = RenderGraph.CreateTexture("SceneColor", ColorDesc);

    // Add BasePass with our view and draw commands
    FBasePassInputs BasePassInputs;
    BasePassInputs.SceneColor = SceneColor;
    // TODO: Pass ViewInfo and DrawCommands to BasePass when it's ready

    RenderGraph.AddPassObject<FBasePass>(BasePassInputs);

    // TODO: Add lighting pass
    // TODO: Add post-process passes
}
