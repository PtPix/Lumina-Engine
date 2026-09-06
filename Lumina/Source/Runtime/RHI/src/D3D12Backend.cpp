#include "D3D12Backend.h"
#include "D3D12Common.h"
#include "D3D12CommandContext.h"
#include "D3D12CommandQueue.h"
#include "D3D12DeferredReleaseQueue.h"
#include "D3D12Device.h"
#include "D3D12SwapChain.h"
#include "D3D12BindlessDescriptorHeap.h"
#include "Profiler/Profiler.h"

FD3D12Backend::~FD3D12Backend()
{
    Shutdown();
}

bool FD3D12Backend::Initialize(const FD3D12BackendDesc& Desc)
{
    if (mbInitialized) return true;

    mDesc = Desc;

    if (!CreateDevice()) { Shutdown(); return false; }

    if (!CreateSwapChain()) { Shutdown(); return false; }

    FD3D12DeferredReleaseQueue::Initialize(mpDevice->GetGraphicsCommandQueue());

    mbInitialized = true;
    return true;
}

void FD3D12Backend::Shutdown()
{
    if (!mpDevice && !mpSwapChain)
    {
        mbInitialized = false;
        return;
    }

    FlushAllQueues();

    FD3D12DeferredReleaseQueue::Shutdown();

    DestroySwapChain();
    mpDevice.reset();

    mbInitialized = false;
}

void FD3D12Backend::FlushAllQueues()
{
    if (!mpDevice) return;

    if (FD3D12CommandQueue* pGraphicsQueue = mpDevice->GetGraphicsCommandQueue())
    {
        pGraphicsQueue->Flush();
    }
    if (FD3D12CommandQueue* pComputeQueue = mpDevice->GetComputeCommandQueue())
    {
        pComputeQueue->Flush();
    }
    if (FD3D12CommandQueue* pCopyQueue = mpDevice->GetCopyCommandQueue())
    {
        pCopyQueue->Flush();
    }
}

void FD3D12Backend::CollectGarbage()
{
    // Process deferred descriptor destructions
    if (mpDevice && mpDevice->GetBindlessDescriptorHeap())
    {
        mpDevice->GetBindlessDescriptorHeap()->ReleaseStaleSlots();
    }

    FD3D12DeferredReleaseQueue::Flush();
}

bool FD3D12Backend::ResizeSwapChain(uint32_t Width, uint32_t Height)
{
    if (!mpSwapChain || Width == 0 || Height == 0)
    {
        return false;
    }

    FlushAllQueues();
    FD3D12DeferredReleaseQueue::FlushAll();

    HRESULT HResult = mpSwapChain->Resize(static_cast<int>(Width), static_cast<int>(Height), mDesc.BackBufferFormat);
    if (FAILED(HResult)) return false;

    mDesc.Width = Width;
    mDesc.Height = Height;
    return true;
}

void FD3D12Backend::Present()
{
    if (!mpSwapChain) return;

    mpSwapChain->Present();
    mpSwapChain->MoveToNextFrame();
}

uint32_t FD3D12Backend::GetCurrentBackBufferIndex() const
{
    return mpSwapChain ? mpSwapChain->GetCurrentBackBufferIndex() : 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12Backend::GetCurrentBackBufferRTV() const
{
    return mpSwapChain ? mpSwapChain->GetCurrentBackBufferRTVHandle() : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

FD3D12GpuResource* FD3D12Backend::GetCurrentBackBufferResource() const
{
    return mpSwapChain ? mpSwapChain->GetCurrentRenderTargetResource() : nullptr;
}

ID3D12Device* FD3D12Backend::GetD3D12Device() const
{
    return mpDevice ? mpDevice->GetDevice() : nullptr;
}

D3D12MA::Allocator* FD3D12Backend::GetAllocator() const
{
    return mpDevice ? mpDevice->GetAllocator() : nullptr;
}

FD3D12CommandQueue* FD3D12Backend::GetGraphicsQueue() const
{
    return mpDevice ? mpDevice->GetGraphicsCommandQueue() : nullptr;
}

FD3D12CommandQueue* FD3D12Backend::GetComputeQueue() const
{
    return mpDevice ? mpDevice->GetComputeCommandQueue() : nullptr;
}

FD3D12CommandQueue* FD3D12Backend::GetCopyQueue() const
{
    return mpDevice ? mpDevice->GetCopyCommandQueue() : nullptr;
}

FD3D12CommandContext* FD3D12Backend::AllocateGraphicsContext()
{
    return GetGraphicsQueue() ? GetGraphicsQueue()->AllocateContext() : nullptr;
}

FD3D12CommandContext* FD3D12Backend::AllocateComputeContext()
{
    return GetComputeQueue() ? GetComputeQueue()->AllocateContext() : nullptr;
}

FD3D12CommandContext* FD3D12Backend::AllocateCopyContext()
{
    return GetCopyQueue() ? GetCopyQueue()->AllocateContext() : nullptr;
}

uint64_t FD3D12Backend::ExecuteGraphicsContext(FD3D12CommandContext* pCommandContext)
{
    return (GetGraphicsQueue() && pCommandContext) ? GetGraphicsQueue()->ExecuteCommandContext(pCommandContext) : 0;
}

uint64_t FD3D12Backend::ExecuteComputeContext(FD3D12CommandContext* pCommandContext)
{
    return (GetComputeQueue() && pCommandContext) ? GetComputeQueue()->ExecuteCommandContext(pCommandContext) : 0;
}

uint64_t FD3D12Backend::ExecuteCopyContext(FD3D12CommandContext* pCommandContext)
{
    return (GetCopyQueue() && pCommandContext) ? GetCopyQueue()->ExecuteCommandContext(pCommandContext) : 0;
}

FD3D12DescriptorAllocator* FD3D12Backend::GetSrvUavCbvAllocator() const
{
    return mpDevice ? mpDevice->GetSRVAllocator() : nullptr;
}

FD3D12DescriptorAllocator* FD3D12Backend::GetRtvAllocator() const
{
    return mpDevice ? mpDevice->GetRTVAllocator() : nullptr;
}

FD3D12DescriptorAllocator* FD3D12Backend::GetDsvAllocator() const
{
    return mpDevice ? mpDevice->GetDSVAllocator() : nullptr;
}

FD3D12BindlessDescriptorHeap* FD3D12Backend::GetBindlessDescriptorHeap() const
{
    return mpDevice ? mpDevice->GetBindlessDescriptorHeap() : nullptr;
}

bool FD3D12Backend::CreateDevice()
{
    LUMINA_TIME_LOG_SCOPE("Create FDevice & Core Infra");

    mpDevice = std::make_unique<FD3D12Device>();

    FDeviceCreateDesc DeviceCreateDesc = {};
    DeviceCreateDesc.bEnableDebugLayer = mDesc.bEnableDebugLayer;
    DeviceCreateDesc.bEnableValidationLayer = mDesc.bEnableGpuValidation;
    DeviceCreateDesc.pFactory = mDesc.ExternalFactory;

    return mpDevice->Create(DeviceCreateDesc);
}

bool FD3D12Backend::CreateSwapChain()
{
    LUMINA_TIME_LOG_SCOPE("Create Swap chain");

    if (!mpDevice)
    {
        return false;
    }

    mpSwapChain = std::make_unique<FD3D12SwapChain>();

    FSwapChainCreateDesc SwapChainDesc = {};
    SwapChainDesc.pDevice = mpDevice.get();
    SwapChainDesc.pCommandQueue = mpDevice->GetGraphicsCommandQueue();
    SwapChainDesc.Hwnd = mDesc.Hwnd;
    SwapChainDesc.WindowWidth = mDesc.Width;
    SwapChainDesc.WindowHeight = mDesc.Height;
    SwapChainDesc.NumBackBuffers = static_cast<int>(mDesc.BackBufferCount);
    SwapChainDesc.bVSync = mDesc.bVSync;

    return mpSwapChain->Create(SwapChainDesc);
}

void FD3D12Backend::DestroySwapChain()
{
    mpSwapChain.reset();
}
