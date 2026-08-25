#include "Renderer/Renderer.h"

#include "../include/Renderer/D3D12/D3D12Backend.h"
#include "../include/Renderer/D3D12/D3D12CommandContext.h"
#include "../include/Renderer/D3D12/D3D12Device.h"
#include "../include/Renderer/D3D12/D3D12SwapChain.h"
#include "../include/Renderer/D3D12/D3D12BindlessDescriptorHeap.h"

#include "Renderer/Managers/TextureManager.h"
#include "Renderer/Scene/FSceneView.h"

#include <cassert>

#include "../include/Renderer/D3D12/D3D12DeferredReleaseQueue.h"
#include "Renderer/Pipeline/GlobalRootSignature.h"
#include "Renderer/Pipeline/PipelineStateCache.h"
#include "Renderer/Pipeline/ShaderManager.h"

std::unique_ptr<FD3D12Backend> Renderer::mpD3D12Backend = nullptr;

FD3D12ResourceUploader Renderer::mUploader;
FRenderGraph Renderer::mRenderGraph;
FD3D12CommandContext* Renderer::mpCurrentFrameContext = nullptr;

FRGTextureHandle Renderer::mBackBufferHandle = { UINT32_MAX };

bool Renderer::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    FRendererInitParams CoreParams = {};
    CoreParams.WindowHandle = Hwnd;
    CoreParams.Width = Width;
    CoreParams.Height = Height;
    CoreParams.bEnableDebugLayer = true;

    if (!FRendererCore::Initialize(CoreParams))
    {
        return false;
    }

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

    FD3D12DeferredReleaseQueue::FlushAll();

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
    FRendererCore::BeginFrame();
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

    FRendererCore::EndFrame();
}

FMesh* Renderer::CreateMesh(const FMeshData& CpuData)
{
    auto* pMesh = new FMesh();

    pMesh->Initialize(CpuData, mpD3D12Backend->GetAllocator(), &mUploader);
    // TODO : Streaming

    return pMesh;
}

void Renderer::BindGlobalResources(FD3D12CommandContext* pContext)
{
    if (!pContext) return;

    FD3D12BindlessDescriptorHeap* pHeap = mpD3D12Backend->GetBindlessDescriptorHeap();
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
