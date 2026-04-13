#include "Samples/ModelApp.h"

#include "Logger/Logger.h"
#include "ImGUI/imgui.h"
#include "Assets/StaticModel.h"
#include "Engine/Input.h"

#include <DirectXMath.h>
#include <string>

bool ModelApp::OnInit()
{
    mCamera.SetLens(DirectX::XM_PIDIV4, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 1000.0f);

    if (!mRenderer.Initialize(mpGraphicsDevice, mWidth, mHeight))
    {
        return false;
    }

    mTextureManager.Initialize(mpGraphicsDevice->GetDevice().GetDevice(), mpGraphicsDevice->GetAllocator(), &mpGraphicsDevice->GetUploadHeap(), &mpGraphicsDevice->GetCbvSrvUavHeap());

    // Initialize Materials
    if (!mBasePassMaterial.Initialize(mpGraphicsDevice->GetDevice().GetDevice(), mRenderer.GetRootSignature()))
    {
        return false;
    }

    if (!mSkyboxMaterial.Initialize(mpGraphicsDevice->GetDevice().GetDevice(), mRenderer.GetRootSignature()))
    {
        return false;
    }

    // Load Models
    mScene.CharacterModel = new StaticModel();
    mScene.SkyboxModel = new StaticModel();
    ResourceUploader Uploader;
    FCommandContext Context;
    Uploader.Initialize(mpGraphicsDevice->GetAllocator(), &mpGraphicsDevice->GetGraphicsContext(), &mpGraphicsDevice->GetGraphicsQueue());
    mScene.CharacterModel->LoadFromFile("Assets/Models/DamagedHelmet/DamagedHelmet.gltf", *mpGraphicsDevice, &Uploader);
    mScene.SkyboxModel->LoadFromFile("Assets/Models/Cube/Cube.gltf", *mpGraphicsDevice, &Uploader);

    const char* TexturePaths[5] = {
        "Assets/Textures/DamagedHelmet/Default_albedo.jpg",
        "Assets/Textures/DamagedHelmet/Default_normal.jpg",
        "Assets/Textures/DamagedHelmet/Default_metalRoughness.jpg",
        "Assets/Textures/DamagedHelmet/Default_emissive.jpg",
        "Assets/Textures/DamagedHelmet/Default_AO.jpg"
    };
    for (int i = 0; i < 5; i++)
    {
        mScene.PBRTextures[i] = mTextureManager.GetOrCreateTextureFromFile(TexturePaths[i]);
        if (!mScene.PBRTextures[i])
        {
            Log::Error("Failed to load texture: %s", TexturePaths[i]);
            return false;
        }
    }

    mpGraphicsDevice->GetCbvSrvUavHeap().AllocateDescriptor(5, &mHelmetPBRTable);
    UINT DescriptorSize = mpGraphicsDevice->GetDevice().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE DestHandle = mHelmetPBRTable.GetCpuDescriptorHandle();

    for (int i = 0; i < 5; i++)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SRVDesc.Format = mScene.PBRTextures[i]->Format;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;

        mpGraphicsDevice->GetDevice().GetDevice()->CreateShaderResourceView(mScene.PBRTextures[i]->Resource, &SRVDesc, DestHandle);
        DestHandle.ptr += DescriptorSize;
    }

    mScene.SkyboxTexture = mTextureManager.GetOrCreateTextureFromFile("Assets/Textures/Environments/Skybox.hdr");

    mBasePassMaterial.SetSrvTable(mHelmetPBRTable.GetGpuDescriptorHandle());
    mSkyboxMaterial.SetSrvTable(mScene.SkyboxTexture->SourceView.GetGpuDescriptorHandle());

    size_t HelmetIndex = mScene.CharacterModel->SumbitToScene(&mScene, DirectX::XMMatrixIdentity());
    size_t HelmetEndIndex = mScene.GetRenderNodes().size();
    for (size_t i = HelmetIndex; i < HelmetEndIndex; ++i)
    {
        mScene.GetRenderNodes()[i].pMaterial = &mBasePassMaterial;
    }

    DirectX::XMMATRIX SkyTransform = DirectX::XMMatrixScaling(500.0f, 500.0f, 500.0f);
    size_t SkyboxIndex = mScene.SkyboxModel->SumbitToScene(&mScene, SkyTransform);
    for (size_t i = SkyboxIndex; i < mScene.GetRenderNodes().size(); ++i)
    {
        mScene.GetRenderNodes()[i].pMaterial = &mSkyboxMaterial;
    }

    Uploader.FlushAndSync(mpGraphicsDevice->GetDevice().GetDevice());
    mpGraphicsDevice->GetUploadHeap().UploadToGPUAndWait(mpGraphicsDevice->GetGraphicsQueue().GetCommandQueue());

    return true;
}

void ModelApp::OnUpdate(double DeltaTime)
{
    mScene.MainLight.Direction = mLightDirection;
    mScene.MainLight.Color = mLightColor;
    mScene.Metallic = mMetallic;
    mScene.Roughness = mRoughness;
    mScene.AO = mAO;

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

    mView = mCamera.GetSceneView(mWidth, mHeight);
}

void ModelApp::OnRender(ID3D12GraphicsCommandList* pCommandList)
{
    mRenderer.Render(pCommandList, mView, mScene);
}

void ModelApp::OnRenderUI()
{
    // 1. 获取主显示器视口 (Viewport)
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // 2. 设置底层背景窗口的属性：没有标题栏、不能折叠、不能调整大小、不能移动、没有背景色
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // 取消窗口圆角和边距，使其完全贴合屏幕边缘
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    // 3. 开始绘制这个底层窗口
    ImGui::Begin("LuminaEditor_Background", nullptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3); // 恢复样式设置

    // 4. 声明 DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("LuminaDockSpace");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    ImGui::End();

    ImGui::Begin("PBR Engine Control");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();

    ImGui::Text("Light Settings");
    ImGui::SliderFloat3("Light Dir", &mLightDirection.x, -1.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", &mLightColor.x);

    ImGui::Separator();
    ImGui::Text("PBR Material");

    ImGui::SliderFloat("Metallic", &mMetallic, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &mRoughness, 0.01f, 1.0f);
    ImGui::SliderFloat("AO", &mAO, 0.01f, 1.0f);

    ImGui::End();
}

void ModelApp::OnDestroy()
{
    if (mScene.CharacterModel)
    {
        delete mScene.CharacterModel;
        mScene.CharacterModel = nullptr;
    }

    if (mScene.SkyboxModel)
    {
        delete mScene.SkyboxModel;
        mScene.SkyboxModel = nullptr;
    }

    mBasePassMaterial.Destroy();
    mSkyboxMaterial.Destroy();
    mTextureManager.DestroyAll();
    mRenderer.Destroy();
}
