#include "Renderer/Renderer.h"

#include "Renderer/RenderTypes.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Core/SwapChain.h"
#include "Renderer/D3D12Core/Descriptors/BindlessDescriptorHeap.h"

#include "Renderer/Managers/TextureManager.h"
#include "Renderer/Scene/FSceneView.h"
#include "Renderer/RendererCommon.h"

#include <cassert>

std::unique_ptr<FD3D12Backend> Renderer::mpD3D12Backend = nullptr;
std::unique_ptr<FBasePass> Renderer::mBasePass = nullptr;

FRootSignature Renderer::mBindlessRootSignature;
FResourceUploader Renderer::mUploader;
FRenderGraph Renderer::mRenderGraph;
FCommandContext* Renderer::mpCurrentFrameContext = nullptr;

FRGTextureHandle Renderer::mBackBufferHandle = { UINT32_MAX };
FFrameResource Renderer::mFrameResources[Renderer::NUM_FRAMES];
uint32_t Renderer::mCurrentFrameIndex = 0;

bool Renderer::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    mpD3D12Backend = std::make_unique<FD3D12Backend>();
    FD3D12BackendDesc D3D12BackendDesc;
    D3D12BackendDesc.Hwnd = Hwnd;
    D3D12BackendDesc.Width = Width;
    D3D12BackendDesc.Height = Height;

    mpD3D12Backend->Initialize(D3D12BackendDesc);

    mRenderGraph.Initialize(mpD3D12Backend->GetDevice(), mpD3D12Backend->GetAllocator());

    mUploader.Initialize(mpD3D12Backend->GetDevice());

    // mUploader.BeginUpload();
    TextureManager::Initialize(mpD3D12Backend->GetDevice(), &mUploader);
    // mUploader.EndUpLoadAndExecute();
    mUploader.FlushAndSync();

    InitializeBindlessRootSignature();
    InitializeSceneBuffers();

    mBasePass = std::make_unique<FBasePass>();
    mBasePass->Initialize(mpD3D12Backend->GetDevice());

    return true;
}

void Renderer::Shutdown()
{
    if (mpD3D12Backend)
        mpD3D12Backend->FlushAllQueues();

    mRenderGraph.Shutdown();
    mUploader.FlushAndSync();

    if (mBasePass) {
        mBasePass->Shutdown();
        mBasePass.reset();
    }

    TextureManager::Shutdown();
    DestroySceneBuffers();

    mpD3D12Backend->Shutdown();
    mpD3D12Backend.reset();
}

void Renderer::BeginFrame()
{
    mCurrentFrameIndex = mpD3D12Backend->GetCurrentBackBufferIndex();

    mpD3D12Backend->CollectGarbage();
    mUploader.SubmitPendingUploads();
    mUploader.CleanUpStaleUploads();

    mpCurrentFrameContext = mpD3D12Backend->AllocateGraphicsContext();
    // assert(pContext != nullptr && "Failed to allocate FCommandContext for BeginFrame!");

    mRenderGraph.Reset();

    mBackBufferHandle = mRenderGraph.ImportBackBuffer(
        "BackBuffer",
        mpD3D12Backend->GetCurrentBackBufferResource(),
        mpD3D12Backend->GetCurrentBackBufferRTV(),
        mpD3D12Backend->GetWidth(),
        mpD3D12Backend->GetHeight(),
        mpD3D12Backend->GetBackBufferFormat(),
        D3D12_RESOURCE_STATE_PRESENT
    );
}

void Renderer::EndFrame()
{
    if (!mpCurrentFrameContext) return;

    mRenderGraph.Compile();
    mRenderGraph.Execute(mpCurrentFrameContext);

    mpCurrentFrameContext->TransitionResource(mpD3D12Backend->GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT);
    mpCurrentFrameContext->FlushResourceBarriers();

    mpD3D12Backend->ExecuteGraphicsContext(mpCurrentFrameContext);
    mpD3D12Backend->Present();
}

FMesh* Renderer::CreateMesh(const FMeshData& CpuData)
{
    auto* pMesh = new FMesh();

    pMesh->Initialize(CpuData, mpD3D12Backend->GetAllocator(), &mUploader);
    // TODO : Streaming

    return pMesh;
}

void Renderer::InitializeSceneBuffers()
{
    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        mFrameResources[i].Initialize(mpD3D12Backend->GetAllocator(), 10000, 1000);
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

void Renderer::RenderSceneView(class FCommandContext* pContext, const FSceneView& View)
{
    mCurrentFrameIndex = mpD3D12Backend->GetCurrentBackBufferIndex();// GetSwapChain()->GetCurrentBackBufferIndex();
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


    pContext->SetGraphicsRootSignature(GetBindlessRootSignature()->Get());
    ID3D12DescriptorHeap* ppHeaps[] = { mpD3D12Backend->GetBindlessDescriptorHeap()->GetDescriptorHeap() };
    pContext->SetDescriptorHeaps(1, ppHeaps);

    pContext->SetGraphicsRootDescriptorTable(ToRootIndex(ERootParam::BindlessTable), mpD3D12Backend->GetBindlessDescriptorHeap()->GetGpuHandle(0));
    pContext->SetGraphicsRootConstantBufferView(ToRootIndex(ERootParam::GlobalCBV), CurrentFrame.GlobalPassBuffer.GetGPUVirtualAddress());
    pContext->GetCommandList()->SetGraphicsRootShaderResourceView(ToRootIndex(ERootParam::InstanceSRV), CurrentFrame.InstanceBuffer.GetGPUVirtualAddress());
    pContext->GetCommandList()->SetGraphicsRootShaderResourceView(ToRootIndex(ERootParam::MaterialSRV), CurrentFrame.MaterialBuffer.GetGPUVirtualAddress());
    if (mBasePass)
    {
        mBasePass->Execute(pContext, View);
    }
}

void Renderer::OnResize(uint32_t Width, uint32_t Height)
{
    if (mpD3D12Backend)
    {
        mpD3D12Backend->ResizeSwapChain(Width, Height);
    }
}

void Renderer::InitializeBindlessRootSignature()
{
    FRootSignatureBuilder Builder;

    // ERootParam::PerObjectConstant (0) — b0, space0
    Builder.AddRootConstants(0, 0, 1);

    // ERootParam::BindlessTable (1) — SRV table: t0 space1 + t0 space2
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

    // ERootParam::GlobalCBV (2) — b1, space0
    Builder.AddConstantBufferView(1, 0);

    // ERootParam::InstanceSRV (3) — t0, space0
    Builder.AddShaderResourceView(0, 0);

    // ERootParam::MaterialSRV (4) — t1, space0
    Builder.AddShaderResourceView(1, 0);

    Builder.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC);
    Builder.AllowInputLayout();
    Builder.Build(mpD3D12Backend->GetDevice()->GetDevice(), mBindlessRootSignature);
}
