#include "Renderer/Renderer.h"

#include "Renderer/RenderTypes.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Core/SwapChain.h"
#include "Renderer/D3D12Core/Descriptors/BindlessDescriptorHeap.h"

#include "Renderer/Managers/TextureManager.h"
#include "Renderer/Scene/FSceneView.h"

#include <cassert>

#include "Renderer/D3D12Core/Core/DeferredReleaseQueue.h"
#include "Renderer/Pipeline/GlobalRootSignature.h"
#include "Renderer/Pipeline/PipelineStateCache.h"
#include "Renderer/Pipeline/ShaderManager.h"

std::unique_ptr<FD3D12Backend> Renderer::mpD3D12Backend = nullptr;

FResourceUploader Renderer::mUploader;
FRenderGraph Renderer::mRenderGraph;
FCommandContext* Renderer::mpCurrentFrameContext = nullptr;

FRGTextureHandle Renderer::mBackBufferHandle = { UINT32_MAX };

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

    TextureManager::Initialize(mpD3D12Backend->GetDevice(), &mUploader);

    mUploader.FlushAndSync();

    InitializeBindlessRootSignature();

    FPipelineStateCache::Initialize(mpD3D12Backend->GetDevice());

    return true;
}

void Renderer::Shutdown()
{
    if (mpD3D12Backend)
        mpD3D12Backend->FlushAllQueues();

    FDeferredReleaseQueue::FlushAll();

    mRenderGraph.Shutdown();
    mUploader.FlushAndSync();

    FGlobalRootSignature::Shutdown();

    FPipelineStateCache::Shutdown();
    FShaderManager::Clear();

    TextureManager::Shutdown();

    mpD3D12Backend->Shutdown();
    mpD3D12Backend.reset();
}

void Renderer::BeginFrame()
{
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
    if (!mpCurrentFrameContext)
    {
        LUMINA_LOG_ERROR(RHI, "Renderer::EndFrame: no valid command context，Jump over current frame");
        return;
    }

    BindGlobalResources(mpCurrentFrameContext);

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

void Renderer::BindGlobalResources(FCommandContext* pContext)
{
    if (!pContext) return;

    FBindlessDescriptorHeap* pHeap = mpD3D12Backend->GetBindlessDescriptorHeap();
    if (!pHeap) return;

    ID3D12DescriptorHeap* ppHeaps[] = { pHeap->GetDescriptorHeap() };
    pContext->SetDescriptorHeaps(1, ppHeaps);

    // Graphics
    ID3D12RootSignature* pGraphicsRS = FGlobalRootSignature::GetGraphicsRootSignature();
    if (pGraphicsRS)
    {
        pContext->SetGraphicsRootSignature(pGraphicsRS);
        pContext->SetGraphicsRootDescriptorTable(ToRootIndex(EGlobalRootParam::BindlessTable), pHeap->GetGpuHandle(0));
    }

    // Compute
    ID3D12RootSignature* pComputeRS = FGlobalRootSignature::GetComputeRootSignature();
    if (pComputeRS)
    {
        pContext->SetComputeRootSignature(pComputeRS);
        pContext->SetComputeRootDescriptorTable(ToRootIndex(EGlobalRootParam::BindlessTable), pHeap->GetGpuHandle(0));
    }
}

void Renderer::OnResize(uint32_t Width, uint32_t Height)
{
    if (mpD3D12Backend)
    {
        mpD3D12Backend->FlushAllQueues();
        mRenderGraph.ClearImportedResources();
        mpD3D12Backend->ResizeSwapChain(Width, Height);
    }
}

void Renderer::InitializeBindlessRootSignature()
{
    if (!FGlobalRootSignature::Initialize(mpD3D12Backend->GetDevice()))
    {
        LUMINA_LOG_ERROR(RHI, "Global RootSignature initialization failed.");
    }
}
