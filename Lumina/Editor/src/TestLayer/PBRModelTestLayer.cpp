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

    uint32_t MaterialID = static_cast<uint32_t>(mSceneMaterials.size());
    RadioMaterial.SetMaterialID(MaterialID);

    mSceneMaterials.push_back(RadioMaterial);
    mMaterialCache.push_back(RadioMaterial.GetMaterialData());

    StaticModel Radio;
    Radio.LoadFromFile("Assets/Models/Radio/SM_HandRadio.fbx");
    const auto& RadioMeshes = Radio.GetMeshesData();

    for (int i = 0; i < RadioMeshes.size(); i++)
    {
        FMesh* pGpuMesh = Renderer::CreateMesh(RadioMeshes[i]);
        mLoadedMeshes.push_back(pGpuMesh);

        CreateInstanceInScene(pGpuMesh, DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f), MaterialID);
    }
    mInstanceBuffer.Create(D3D12Backend::GetAllocator(), sizeof(FInstanceData), 1000,
                               D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, L"GlobalInstanceBuffer");

    mMaterialBuffer.Create(D3D12Backend::GetAllocator(), sizeof(FPBRMaterialData), 100,
                           D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, L"GlobalMaterialBuffer");

    memcpy(mMaterialBuffer.Map(), mMaterialCache.data(), sizeof(FPBRMaterialData) * mMaterialCache.size());
    mMaterialBuffer.Unmap();

    mGlobalPassBuffer.Create(
            D3D12Backend::GetAllocator(),
            sizeof(FGlobalPassData),
            L"GlobalPassBuffer"
        );

    // 3. 执行持久映射
    mpMappedGlobalData = mGlobalPassBuffer.Map();

    // 填入一些初始默认数据
    mGlobalDataCache.SunDirection = { 0.577f, -0.577f, 0.577f }; // 假设阳光从右上角照下
    mGlobalDataCache.SunColor = { 1.0f, 0.9f, 0.8f, 1.0f };      // 暖色阳光
    mGlobalDataCache.SunIntensity = 3.14f;

    memcpy(mpMappedGlobalData, &mGlobalDataCache, sizeof(FGlobalPassData));

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

    mInstanceBuffer.Destroy();
    mMaterialBuffer.Destroy();

    for (auto& LoadedMesh : mLoadedMeshes)
    {
        LoadedMesh->Destroy();
    }

    mSceneObjects.clear();
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

    // 2. 🚨 必须转置矩阵以适配 HLSL 的列主序
    mGlobalDataCache.ViewProjectionMatrix = DirectX::XMMatrixTranspose(viewProj);

    mGlobalDataCache.CameraPosition = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

    // 如果太阳光方向随时间变化，也可以在这里更新
    // ...

    // 3. 直接使用 memcpy 将 CPU 数据推送到 Upload 显存中
    if (mpMappedGlobalData)
    {
        memcpy(mpMappedGlobalData, &mGlobalDataCache, sizeof(FGlobalPassData));
    }

    if (!mInstanceData.empty())
    {
        void* pMappedInstances = mInstanceBuffer.Map();
        memcpy(pMappedInstances, mInstanceData.data(), sizeof(FInstanceData) * mInstanceData.size());
        mInstanceBuffer.Unmap();
    }
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

    pCommandContext->SetGraphicsRootSignature(Renderer::GetBindlessRootSignature()->Get());
    pCommandContext->SetPipelineState(mBasePassPSO.Get());
    pCommandContext->SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Bind Bindless Descriptor Heap
    ID3D12DescriptorHeap* ppHeaps[] = { D3D12Backend::GetBindlessDescriptorHeap()->GetDescriptorHeap() };
    pCommandContext->SetDescriptorHeaps(1, ppHeaps);

    // Bind Bindless Descriptor Table and Global Data
    pCommandContext->SetGraphicsRootDescriptorTable(1, D3D12Backend::GetBindlessDescriptorHeap()->GetGpuHandle(0));
    pCommandContext->SetGraphicsRootConstantBufferView(2, mGlobalPassBuffer.GetGPUVirtualAddress());
    pCommandContext->GetCommandList()->SetGraphicsRootShaderResourceView(3, mInstanceBuffer.GetGPUVirtualAddress());
    pCommandContext->GetCommandList()->SetGraphicsRootShaderResourceView(4, mMaterialBuffer.GetGPUVirtualAddress());
    // DrawCall
    for (auto& SceneObject : mSceneObjects)
    {
        pCommandContext->SetGraphicsRoot32BitConstants(0, 1, &SceneObject.InstanceIndex, 0);
        SceneObject.pMesh->Draw(pCommandContext);
    }
}

void PBRModelTestLayer::OnRenderUI()
{
    // ImGui::Begin("PBR Test Layer");

    // ImGui::End();
}

void PBRModelTestLayer::CreateInstanceInScene(FMesh* pMesh, DirectX::XMMATRIX Transform, uint32_t MaterialID)
{
    FSceneObject SceneObject;
    SceneObject.pMesh = pMesh;
    SceneObject.InstanceIndex = static_cast<uint32_t>(mInstanceData.size());

    FInstanceData InstanceData;
    InstanceData.WorldMatrix = DirectX::XMMatrixTranspose(Transform);
    InstanceData.MaterialIndex = MaterialID;

    mInstanceData.push_back(InstanceData);
    mSceneObjects.push_back(SceneObject);
    //
    // FObjectData ObjectData{};
    // ObjectData.WorldMatrix = DirectX::XMMatrixTranspose(Transform);
    // // ObjectData.WorldMatrix = Transform;
    // ObjectData.BaseColor = Color;
    // ObjectData.AlbedoTexIndex = AlbedoIdx;
    // ObjectData.NormalTexIndex = NormalIdx;
    // ObjectData.RMATexIndex = RMAIdx;
    // ObjectData.Padding = 0;
    //
    // SceneObject.ObjectDataBuffer.Create(D3D12Backend::GetAllocator(), sizeof(FObjectData), 256,
    //                              D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
    //                              D3D12_HEAP_TYPE_UPLOAD, L"Node_ObjectData");
    //
    // memcpy(SceneObject.ObjectDataBuffer.Map(), &ObjectData, sizeof(FObjectData));
    // SceneObject.ObjectDataBuffer.Unmap();
    //
    // SceneObject.BindlessIndex = D3D12Backend::GetBindlessDescriptorHeap()->AllocateSlot();
    //
    // D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    // srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    // srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    // srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    // srvDesc.Buffer.FirstElement = 0;
    // srvDesc.Buffer.NumElements = 1;
    // srvDesc.Buffer.StructureByteStride = sizeof(FObjectData);
    //
    // D3D12_CPU_DESCRIPTOR_HANDLE DestHandle = D3D12Backend::GetBindlessDescriptorHeap()->GetCpuHandle(SceneObject.BindlessIndex);
    // D3D12Backend::GetDevice()->GetDevice()->CreateShaderResourceView(SceneObject.ObjectDataBuffer.GetResource(), &srvDesc, DestHandle);
    //
    // mSceneObjects.push_back(std::move(SceneObject));
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

