#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Common.h"

#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/D3D12Core/Core/CommandQueue.h"
#include "Renderer/D3D12Core/Core/DeferredReleaseQueue.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Core/SwapChain.h"
#include "Renderer/D3D12Core/Descriptors/BindlessDescriptorHeap.h"

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

    FDeferredReleaseQueue::Initialize(mpDevice->GetGraphicsCommandQueue());

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

    FDeferredReleaseQueue::Shutdown();

    DestroySwapChain();
    mpDevice.reset();

    mbInitialized = false;
}

void FD3D12Backend::FlushAllQueues()
{
    if (!mpDevice) return;

    if (FCommandQueue* pGraphicsQueue = mpDevice->GetGraphicsCommandQueue())
    {
        pGraphicsQueue->Flush();
    }
    if (FCommandQueue* pComputeQueue = mpDevice->GetComputeCommandQueue())
    {
        pComputeQueue->Flush();
    }
    if (FCommandQueue* pCopyQueue = mpDevice->GetCopyCommandQueue())
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

    FDeferredReleaseQueue::Flush();
}

bool FD3D12Backend::ResizeSwapChain(uint32_t Width, uint32_t Height)
{
    if (!mpSwapChain || Width == 0 || Height == 0)
    {
        return false;
    }

    FlushAllQueues();
    FDeferredReleaseQueue::FlushAll();

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

GpuResource* FD3D12Backend::GetCurrentBackBufferResource() const
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

FCommandQueue* FD3D12Backend::GetGraphicsQueue() const
{
    return mpDevice ? mpDevice->GetGraphicsCommandQueue() : nullptr;
}

FCommandQueue* FD3D12Backend::GetComputeQueue() const
{
    return mpDevice ? mpDevice->GetComputeCommandQueue() : nullptr;
}

FCommandQueue* FD3D12Backend::GetCopyQueue() const
{
    return mpDevice ? mpDevice->GetCopyCommandQueue() : nullptr;
}

FCommandContext* FD3D12Backend::AllocateGraphicsContext()
{
    return GetGraphicsQueue() ? GetGraphicsQueue()->AllocateContext() : nullptr;
}

FCommandContext* FD3D12Backend::AllocateComputeContext()
{
    return GetComputeQueue() ? GetComputeQueue()->AllocateContext() : nullptr;
}

FCommandContext* FD3D12Backend::AllocateCopyContext()
{
    return GetCopyQueue() ? GetCopyQueue()->AllocateContext() : nullptr;
}

uint64_t FD3D12Backend::ExecuteGraphicsContext(FCommandContext* pCommandContext)
{
    return (GetGraphicsQueue() && pCommandContext) ? GetGraphicsQueue()->ExecuteCommandContext(pCommandContext) : 0;
}

uint64_t FD3D12Backend::ExecuteComputeContext(FCommandContext* pCommandContext)
{
    return (GetComputeQueue() && pCommandContext) ? GetComputeQueue()->ExecuteCommandContext(pCommandContext) : 0;
}

uint64_t FD3D12Backend::ExecuteCopyContext(FCommandContext* pCommandContext)
{
    return (GetCopyQueue() && pCommandContext) ? GetCopyQueue()->ExecuteCommandContext(pCommandContext) : 0;
}

FDescriptorAllocator* FD3D12Backend::GetSrvUavCbvAllocator() const
{
    return mpDevice ? mpDevice->GetSRVAllocator() : nullptr;
}

FDescriptorAllocator* FD3D12Backend::GetRtvAllocator() const
{
    return mpDevice ? mpDevice->GetRTVAllocator() : nullptr;
}

FDescriptorAllocator* FD3D12Backend::GetDsvAllocator() const
{
    return mpDevice ? mpDevice->GetDSVAllocator() : nullptr;
}

FBindlessDescriptorHeap* FD3D12Backend::GetBindlessDescriptorHeap() const
{
    return mpDevice ? mpDevice->GetBindlessDescriptorHeap() : nullptr;
}

bool FD3D12Backend::CreateDevice()
{
    LUMINA_TIME_LOG_SCOPE("Create FDevice & Core Infra");

    mpDevice = std::make_unique<FDevice>();

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

    mpSwapChain = std::make_unique<FSwapChain>();

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
