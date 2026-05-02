#include "Renderer/Renderer.h"

#include "Renderer/RenderTypes.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Resource/FResourceUploader.h"
#include "Renderer/Managers/FTextureManager.h"
#include "Renderer/Scene/FSceneView.h"

FRootSignature Renderer::mBindlessRootSignature;
FResourceUploader Renderer::mUploader;

FFrameResource Renderer::mFrameResources[Renderer::NUM_FRAMES];
uint32_t Renderer::mCurrentFrameIndex = 0;

bool Renderer::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    D3D12Backend::Initialize(Hwnd, Width, Height);

    mUploader.Initialize(D3D12Backend::GetDevice());

    mUploader.BeginUpload();
    TextureManager::Initialize(D3D12Backend::GetDevice(), &mUploader);
    mUploader.EndUpLoadAndExecute();
    mUploader.FlushAndSync();

    InitializeBindlessRootSignature();
    InitializeSceneBuffers();

    return true;
}

void Renderer::Shutdown()
{
    mUploader.FlushAndSync();
    TextureManager::Shutdown();
    DestroySceneBuffers();
    D3D12Backend::Shutdown();
}

FCommandContext* Renderer::BeginFrame()
{
    D3D12Backend::BeginFrame();

    mUploader.CleanUpStaleUploads();

    FCommandContext* pContext = D3D12Backend::AllocateContext();

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    pContext->FlushResourceBarriers();

    return pContext;
}

void Renderer::EndFrame(FCommandContext* pContext)
{
    if (!pContext) return;

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT);
    pContext->FlushResourceBarriers();

    D3D12Backend::ExecuteGraphicsContext(pContext);

    D3D12Backend::EndFrameAndPresent();
}

FMesh* Renderer::CreateMesh(const FMeshData& CpuData)
{
    auto* pMesh = new FMesh();

    mUploader.BeginUpload();
    pMesh->Initialize(CpuData, D3D12Backend::GetAllocator(), &mUploader);
    // TODO : Streaming
    mUploader.FlushAndSync();

    return pMesh;
}

void Renderer::InitializeSceneBuffers()
{
    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        mFrameResources[i].Initialize(10000, 1000);
    }
}

void Renderer::DestroySceneBuffers()
{
    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        mFrameResources[i].GlobalPassBuffer.Destroy();
        mFrameResources[i].InstanceBuffer.Destroy();
        mFrameResources[i].MaterialBuffer.Destroy();
    }
}

void Renderer::RenderSceneView(class FCommandContext* pContext, const FSceneView& View,
    ID3D12PipelineState* pPSO)
{
    mCurrentFrameIndex = D3D12Backend::GetSwapChain()->GetCurrentBackBufferIndex();
    FFrameResource& CurrentFrame = mFrameResources[mCurrentFrameIndex];

    memcpy(CurrentFrame.GlobalPassBuffer.Map(), &View.GlobalPassData, sizeof(FGlobalPassData));
    CurrentFrame.GlobalPassBuffer.Unmap();

    if (!View.InstanceData.empty())
    {
        memcpy(CurrentFrame.InstanceBuffer.Map(), View.InstanceData.data(), sizeof(FInstanceData) * View.InstanceData.size());
        CurrentFrame.InstanceBuffer.Unmap();
    }

    if (!View.MaterialData.empty())
    {
        memcpy(CurrentFrame.MaterialBuffer.Map(), View.MaterialData.data(), sizeof(FPBRMaterialData) * View.MaterialData.size());
        CurrentFrame.MaterialBuffer.Unmap();
    }

    pContext->SetPipelineState(pPSO);
    pContext->SetGraphicsRootSignature(GetBindlessRootSignature()->Get());
    pContext->SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = { D3D12Backend::GetBindlessDescriptorHeap()->GetDescriptorHeap() };
    pContext->SetDescriptorHeaps(1, ppHeaps);

    // ==========================================
    // 3. 绑定我们刚刚上传的大 Buffer 资源 (Root Parameters)
    // ==========================================
    pContext->SetGraphicsRootDescriptorTable(1, D3D12Backend::GetBindlessDescriptorHeap()->GetGpuHandle(0));
    pContext->SetGraphicsRootConstantBufferView(2, CurrentFrame.GlobalPassBuffer.GetGPUVirtualAddress());
    pContext->GetCommandList()->SetGraphicsRootShaderResourceView(3, CurrentFrame.InstanceBuffer.GetGPUVirtualAddress());
    pContext->GetCommandList()->SetGraphicsRootShaderResourceView(4, CurrentFrame.MaterialBuffer.GetGPUVirtualAddress());

    // ==========================================
    // 4. 纯粹、无脑且高效的 DrawCall 发射机！
    // ==========================================
    for (const auto& Cmd : View.DrawCommands)
    {
        // 只需告诉 Shader，去找数组里的第几个 Instance
        pContext->SetGraphicsRoot32BitConstants(0, 1, &Cmd.InstanceIndex, 0);
        Cmd.pMesh->Draw(pContext);
    }
}

void Renderer::InitializeBindlessRootSignature()
{
    RootSignatureBuilder Builder;

    // Parameter 0 : Per Object Bindless Index
    // register(b0, space0)
    // Refers to the object data in descriptor heap
    Builder.AddRootConstants(0, 0, 1);

    // Parameter 1 : Bindless Resource
    // register(t0, space1)
    // Possess all of the SRV, CBV, UAV
    std::vector<D3D12_DESCRIPTOR_RANGE1> BindlessRanges;
    D3D12_DESCRIPTOR_RANGE1 SrvRange = {};
    SrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    SrvRange.NumDescriptors = UINT_MAX;
    SrvRange.BaseShaderRegister = 0;
    SrvRange.RegisterSpace = 1;
    SrvRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
    SrvRange.OffsetInDescriptorsFromTableStart = 0;
    BindlessRanges.push_back(SrvRange);


    D3D12_DESCRIPTOR_RANGE1 BufRange = {};
    BufRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    BufRange.NumDescriptors = UINT_MAX;
    BufRange.BaseShaderRegister = 0;
    BufRange.RegisterSpace = 2;
    BufRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
    BufRange.OffsetInDescriptorsFromTableStart = 0;
    BindlessRanges.push_back(BufRange);

    Builder.AddDescriptorTable(BindlessRanges, D3D12_SHADER_VISIBILITY_ALL);
    // Parameter 2 : Global Static Data
    // register(b1, space0)
    // Root CBV
    Builder.AddConstantBufferView(1, 0);
    Builder.AddShaderResourceView(0, 0);
    Builder.AddShaderResourceView(1, 0);

    // Static Sampler
    // register(s0, space0)
    Builder.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC);

    Builder.AllowInputLayout();

    Builder.Build(D3D12Backend::GetDevice()->GetDevice(), mBindlessRootSignature);
}
