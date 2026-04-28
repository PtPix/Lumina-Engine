#include <d3d12.h>

#include "Editor/TestLayer/PBRModelTestLayer.h"

#include "Assets/StaticModel.h"
#include "Engine/Input.h"
#include "ImGUI/imgui.h"
#include "Logger/Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/GraphicsDevice.h"
#include "Renderer/D3D12Core/ShaderCompiler.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"

void PBRModelTestLayer::OnAttach()
{
    LUMINA_LOG_INFO(App, "PBRModelTestLayer Attaching...");

    StaticModel DamagedHelmetModel;
    DamagedHelmetModel.LoadFromFile("Assets/Models/DamagedHelmet/DamagedHelmet.gltf");

    // TODO : Multiple Meshes support
    const auto& CpuMeshes = DamagedHelmetModel.GetMeshesData();
    FMesh* pGpuMesh = Renderer::CreateMesh(CpuMeshes[0]);

    CreateInstanceInScene(pGpuMesh, DirectX::XMMatrixTranslation(-5.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(1,0,0,1));
    CreateInstanceInScene(pGpuMesh, DirectX::XMMatrixTranslation(5.0f, 0.0f, 0.0f),  DirectX::XMFLOAT4(0,1,0,1));
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
}

void PBRModelTestLayer::OnRender(FCommandContext* pCommandContext)
{
    ID3D12GraphicsCommandList* pCmdList = pCommandContext->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV = D3D12Backend::GetCurrentBackBufferRTV();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDepthBuffer.GetDSV();
    pCmdList->OMSetRenderTargets(1, &backBufferRTV, FALSE, &dsvHandle);

    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    pCmdList->ClearRenderTargetView(backBufferRTV, clearColor, 0, nullptr);
    pCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    float Width = 1280;
    float Height = 720;
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)Width, (LONG)Height };
    pCmdList->RSSetViewports(1, &viewport);
    pCmdList->RSSetScissorRects(1, &scissorRect);
    pCmdList->SetGraphicsRootSignature(Renderer::GetBindlessRootSignature()->Get());
    // Bind Bindless Descriptor Heap
    ID3D12DescriptorHeap* ppHeaps[] = { D3D12Backend::GetBindlessDescriptorHeap()->GetDescriptorHeap() };
    pCommandContext->SetDescriptorHeaps(1, ppHeaps);
    pCmdList->SetGraphicsRootDescriptorTable(1, D3D12Backend::GetBindlessDescriptorHeap()->GetGpuHandle(0));
    pCmdList->SetGraphicsRootConstantBufferView(2, mGlobalPassBuffer.GetGPUVirtualAddress());

    // ==========================================
    // 3. 绑定 BasePass 的管线状态
    // ==========================================
    pCmdList->SetPipelineState(mBasePassPSO.Get());
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    for (auto& SceneObject : mSceneObjects)
    {
        pCommandContext->GetCommandList()->SetGraphicsRoot32BitConstants(0, 1, &SceneObject.BindlessIndex, 0);
        SceneObject.pMesh->Draw(pCommandContext);
    }
}

void PBRModelTestLayer::OnRenderUI()
{
    // ImGui::Begin("PBR Test Layer");

    // ImGui::End();
}

void PBRModelTestLayer::CreateInstanceInScene(FMesh* pMesh, DirectX::XMMATRIX Transform, DirectX::XMFLOAT4 Color)
{
    FSceneObject SceneObject;
    SceneObject.pMesh = pMesh;

    FObjectData ObjectData{};
    ObjectData.WorldMatrix = DirectX::XMMatrixTranspose(Transform);
    // ObjectData.WorldMatrix = Transform;
    ObjectData.BaseColor = Color;

    SceneObject.ObjectDataBuffer.Create(D3D12Backend::GetAllocator(), sizeof(FObjectData), 256,
                                 D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
                                 D3D12_HEAP_TYPE_UPLOAD, L"Node_ObjectData");

    memcpy(SceneObject.ObjectDataBuffer.Map(), &ObjectData, sizeof(FObjectData));
    SceneObject.ObjectDataBuffer.Unmap();

    SceneObject.BindlessIndex = D3D12Backend::GetBindlessDescriptorHeap()->AllocateSlot();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = 1;
    srvDesc.Buffer.StructureByteStride = sizeof(FObjectData);

    D3D12_CPU_DESCRIPTOR_HANDLE DestHandle = D3D12Backend::GetBindlessDescriptorHeap()->GetCpuHandle(SceneObject.BindlessIndex);
    D3D12Backend::GetDevice()->GetDevice()->CreateShaderResourceView(SceneObject.ObjectDataBuffer.GetResource(), &srvDesc, DestHandle);

    mSceneObjects.push_back(std::move(SceneObject));
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

