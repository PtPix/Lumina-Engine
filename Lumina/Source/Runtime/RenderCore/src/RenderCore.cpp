#include "RenderCore.h"
#include "RenderGraph/RenderGraph.h"
#include "D3D12Device.h"
#include "D3D12CommandContext.h"
#include "D3D12ResourceUploader.h"
#include "D3D12BindlessDescriptorHeap.h"
#include "D3D12DeferredReleaseQueue.h"
#include "D3D12MemAlloc.h"
#include "Logger/Logger.h"

// Include D3D12Backend.h here to get complete type for unique_ptr
#include "D3D12Backend.h"
#include "D3D12SwapChain.h"

#include <cassert>

// Define FRenderPassBase here to provide complete type for unique_ptr in RenderGraph
// This matches the definition in RenderGraph.cpp
class FRenderPassBase
{
public:
    virtual ~FRenderPassBase() = default;
};

// Forward declarations
class FGPUScene;
class FMesh;
struct FMeshData;

// Note: The following initializations are now handled by upper layers (Renderer module)
// to avoid RenderCore depending on Renderer layer types

// Static member initialization
bool FRendererCore::mbInitialized = false;
std::unique_ptr<FD3D12Backend> FRendererCore::mpBackend = nullptr;
std::unique_ptr<FD3D12ResourceUploader> FRendererCore::mpUploader = nullptr;
std::unique_ptr<FRenderGraph> FRendererCore::mpRenderGraph = nullptr;
FRGTextureHandle FRendererCore::mBackBufferHandle = { FRGTextureHandle::InvalidIndex };
FGPUScene* FRendererCore::mpGPUScene = nullptr;
FD3D12CommandContext* FRendererCore::mpCurrentFrameContext = nullptr;

uint32_t FRendererCore::mCurrentFrameIndex = 0;
uint64_t FRendererCore::mFrameNumber = 0;

uint32_t FRendererCore::mRenderWidth = 1920;
uint32_t FRendererCore::mRenderHeight = 1080;

FRenderStats FRendererCore::mStats = {};

bool FRendererCore::Initialize(const FRendererInitParams &Params)
{
    if (mbInitialized) return true;

    mRenderWidth = Params.Width;
    mRenderHeight = Params.Height;

    // Initialize Backend
    InitializeBackend(Params);

    // Initialize RenderGraph
    mpRenderGraph = std::make_unique<FRenderGraph>();
    mpRenderGraph->Initialize(mpBackend->GetDevice(), mpBackend->GetAllocator());

    // Initialize Uploader
    mpUploader = std::make_unique<FD3D12ResourceUploader>();
    mpUploader->Initialize(mpBackend->GetDevice());

    // Note: TextureManager, GlobalRootSignature, PipelineStateCache initialization
    // are now handled by Renderer layer (see Renderer.cpp)
    // This avoids RenderCore depending on Renderer module

    mpGPUScene = nullptr;

    mbInitialized = true;

    return true;
}

void FRendererCore::Shutdown()
{
    if (!mbInitialized) return;

    if (mpBackend)
        mpBackend->FlushAllQueues();

    FD3D12DeferredReleaseQueue::FlushAll();

    if (mpRenderGraph)
        mpRenderGraph->Shutdown();

    if (mpUploader)
        mpUploader->FlushAndSync();

    // Note: TextureManager, GlobalRootSignature, PipelineStateCache, ShaderManager shutdown
    // are now handled by Renderer layer (see Renderer.cpp)

    if (mpBackend)
    {
        mpBackend->Shutdown();
        mpBackend.reset();
    }

    mpUploader.reset();
    mpRenderGraph.reset();

    mpBackend = nullptr;
    mbInitialized = false;
}

void FRendererCore::BeginFrame()
{
    assert(mbInitialized && "RendererCore not initialized!");

    mCurrentFrameIndex = (mCurrentFrameIndex + 1) % NUM_FRAMES_IN_FLIGHT;
    mFrameNumber++;

    mStats.Reset();

    // Backend frame management
    mpBackend->CollectGarbage();
    mpUploader->SubmitPendingUploads();
    mpUploader->CleanUpStaleUploads();

    // Allocate command context for this frame
    mpCurrentFrameContext = mpBackend->AllocateGraphicsContext();

    // Reset RenderGraph
    mpRenderGraph->Reset();

    // Import back buffer
    mBackBufferHandle = mpRenderGraph->ImportBackBuffer(
        "BackBuffer",
        mpBackend->GetCurrentBackBufferResource(),
        mpBackend->GetCurrentBackBufferRTV(),
        mpBackend->GetWidth(),
        mpBackend->GetHeight(),
        mpBackend->GetBackBufferFormat(),
        D3D12_RESOURCE_STATE_PRESENT
    );
}

void FRendererCore::EndFrame()
{
    assert(mbInitialized && "RendererCore not initialized!");

    if (!mpCurrentFrameContext)
    {
        LUMINA_LOG_ERROR(RHI, "FRendererCore::EndFrame: no valid command context, jump over current frame");
        return;
    }

    // Note: BindGlobalResources is called by Renderer layer before compiling RenderGraph
    // It's not called here to avoid dependency on Renderer layer types

    // Compile and execute RenderGraph
    mpRenderGraph->Compile();
    mpRenderGraph->Execute(mpCurrentFrameContext);

    // Transition back buffer to present
    mpCurrentFrameContext->TransitionResource(mpBackend->GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT);
    mpCurrentFrameContext->FlushResourceBarriers();

    // Execute and present
    mpBackend->ExecuteGraphicsContext(mpCurrentFrameContext);
    mpBackend->Present();
}

FD3D12Device * FRendererCore::GetDevice()
{
    assert(mbInitialized && mpBackend && "RendererCore not initialized!");
    return mpBackend->GetDevice();
}

D3D12MA::Allocator * FRendererCore::GetAllocator()
{
    assert(mbInitialized && mpBackend && "RendererCore not initialized!");
    return mpBackend->GetAllocator();
}

FRenderGraph* FRendererCore::GetRenderGraph()
{
    assert(mbInitialized && mpRenderGraph && "RendererCore not initialized!");
    return mpRenderGraph.get();
}

FRGTextureHandle FRendererCore::GetBackBufferHandle()
{
    return mBackBufferHandle;
}

// Note: CreateMesh implementation is in Renderer module (Renderer.cpp)
// to avoid RenderCore depending on Renderer layer types

void FRendererCore::OnResize(uint32_t Width, uint32_t Height)
{
    if (mpBackend)
    {
        mpBackend->FlushAllQueues();
        mpRenderGraph->ClearImportedResources();
        mpBackend->ResizeSwapChain(Width, Height);
    }
}

void FRendererCore::SetRenderResolution(uint32_t Width, uint32_t Height)
{
    mRenderWidth = Width;
    mRenderHeight = Height;
}

// Note: BindGlobalResources implementation is in Renderer module
// to avoid RenderCore depending on Renderer layer types (FGlobalRootSignature, etc.)

void FRendererCore::InitializeBackend(const FRendererInitParams& Params)
{
    mpBackend = std::make_unique<FD3D12Backend>();

    FD3D12BackendDesc D3D12BackendDesc;
    D3D12BackendDesc.Hwnd = Params.WindowHandle;
    D3D12BackendDesc.Width = Params.Width;
    D3D12BackendDesc.Height = Params.Height;

    mpBackend->Initialize(D3D12BackendDesc);
}

// Note: InitializeBindlessRootSignature implementation moved to Renderer layer
// to avoid RenderCore depending on Renderer module types
