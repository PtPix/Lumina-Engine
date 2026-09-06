#include "PbrModelDemo.h"

#include "Assets/StaticModel.h"
#include "ImGUI/imgui.h"
#include "Platform/InputState.h"
#include "Renderer.h"
#include "RenderCore.h"
#include "D3D12Backend.h"
#include "Resources/TextureManager.h"
#include "Passes/BasePass/BasePass.h"
#include "Passes/DebugPasses/DebugVisualizePass.h"
#include "FMesh.h"
#include "FMaterial.h"

void PbrModelDemo::OnAttach()
{
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

void PbrModelDemo::OnDetach()
{
    FRendererCore::GetBackend()->FlushAllQueues();

    for (auto& LoadedMesh : mLoadedMeshes)
    {
        LoadedMesh->Destroy();
        delete LoadedMesh;
    }

    mLoadedMeshes.clear();

    mScene.Clear();
}

void PbrModelDemo::OnUpdate(double DeltaTime)
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
}

void PbrModelDemo::OnRender(FRenderGraph& Graph)
{
    // TODO: Implement rendering with new architecture
    // This needs to be coordinated with the Renderer module's SceneView system
}

void PbrModelDemo::OnRenderUI()
{
    ImGui::SeparatorText("Lighting");
    ImGui::SliderFloat3("Sun Direction", mParams.SunDirection, -1.0f, 1.0f);
    ImGui::ColorEdit3 ("Sun Color",     mParams.SunColor);
    ImGui::SliderFloat("Sun Intensity", &mParams.SunIntensity, 0.0f, 10.0f);
    ImGui::Checkbox   ("Rotate Model",  &mParams.bRotateModel);
}
