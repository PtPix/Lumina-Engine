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

    // uint32_t MaterialID = static_cast<uint32_t>(mSceneMaterials.size());
    // RadioMaterial.SetMaterialID(MaterialID);
    //
    // mSceneMaterials.push_back(RadioMaterial);
    // mMaterialCache.push_back(RadioMaterial.GetMaterialData());

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

    InitBasePassPipeline();
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

    // 2. 清理全局缓冲和深度缓冲
    mGlobalPassBuffer.Destroy();
    mDepthBuffer.Destroy();

    for (auto& LoadedMesh : mLoadedMeshes)
    {
        LoadedMesh->Destroy();
    }

    // mSceneObjects.clear();
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

    Renderer::RenderSceneView(pCommandContext, mSceneView, mBasePassPSO.Get());
}

void PBRModelTestLayer::OnRenderUI()
{
    // ImGui::Begin("PBR Test Layer");

    // ImGui::End();
}

bool PBRModelTestLayer::InitBasePassPipeline()
{
    std::string ErrorString;
    ShaderUtils::FBlob VertexShaderBlob;
    ShaderUtils::FBlob PixelShaderBlob;

    // 1. 编译 Vertex Shader
    FShaderStageCompileDesc VertexShaderStageDesc = {
        L"Shaders/BasePass.hlsl", "VSMain", EShaderStage::VertexShader, EShaderModel::SM6_0
    };
    VertexShaderBlob = ShaderUtils::CompileFromSource(VertexShaderStageDesc, ErrorString);
    if (VertexShaderBlob.IsNull())
    {
        Log::Error("BasePass VS Compile Failed: %s", ErrorString.c_str());
        return false;
    }

    // 2. 编译 Pixel Shader
    FShaderStageCompileDesc PixelShaderStageDesc = {
        L"Shaders/BasePass.hlsl", "PSMain", EShaderStage::PixelShader, EShaderModel::SM6_0
    };
    PixelShaderBlob = ShaderUtils::CompileFromSource(PixelShaderStageDesc, ErrorString);
    if (PixelShaderBlob.IsNull())
    {
        Log::Error("BasePass PS Compile Failed: %s", ErrorString.c_str());
        return false;
    }

    // 3. 配置输入布局 (与 FStandardVertex 对应)
    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 4. 构建 Graphics Pipeline State
    GraphicsPipelineStateBuilder Builder;
    Builder.SetRootSignature(Renderer::GetBindlessRootSignature()->Get())
           .SetInputLayout(InputElements)
           // BasePass 专注于 RT 的输出，这里填入你 G-Buffer 或后备缓冲的格式
           .SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM })
           .SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT);

    Builder.EnableDepthTest(); // 开启深度测试以保证正确的遮挡关系

    if (!VertexShaderBlob.IsNull())
    {
        Builder.SetVertexShader(VertexShaderBlob.GetByteCode(), VertexShaderBlob.GetByteCodeSize());
    }
    if (!PixelShaderBlob.IsNull())
    {
        Builder.SetPixelShader(PixelShaderBlob.GetByteCode(), PixelShaderBlob.GetByteCodeSize());
    }

    // 假设 mBasePassPSO 是 ID3D12PipelineState* 的封装类
    Builder.Build(D3D12Backend::GetDevice()->GetDevice(), mBasePassPSO);

    if (!mBasePassPSO.Get())
    {
        LUMINA_LOG_ERROR(Material, "Failed to build BasePass Pipeline State!");
        return false;
    }

    return true;
}

