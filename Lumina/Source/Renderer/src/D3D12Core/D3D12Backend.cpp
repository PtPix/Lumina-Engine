#include "Renderer/D3D12Core/D3D12Backend.h"

#include "Renderer/D3D12Core/Common.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Core/FSwapChain.h"
#include "Renderer/D3D12Core/Core/FCommandQueue.h"

std::unique_ptr<FDevice> D3D12Backend::mpDevice;
std::unique_ptr<FSwapChain> D3D12Backend::mpSwapChain;

bool D3D12Backend::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    // Create FDevice
    {
        LUMINA_TIME_LOG_SCOPE("Create FDevice & Core Infra");
        mpDevice = std::make_unique<FDevice>();

        FDeviceCreateDesc DeviceCreateDesc = {true, false, nullptr};
        if (!mpDevice->Create(DeviceCreateDesc))
        {
            return false;
        }
    }

    // Create FSwapChain
    {
        LUMINA_TIME_LOG_SCOPE("Create Swap chain");
        mpSwapChain = std::make_unique<FSwapChain>();

        FSwapChainCreateDesc swapChainDesc = {};
        swapChainDesc.pDevice = mpDevice.get();
        swapChainDesc.pCommandQueue = mpDevice->GetGraphicsCommandQueue();
        swapChainDesc.Hwnd = Hwnd;
        swapChainDesc.WindowHeight = Height;
        swapChainDesc.WindowWidth = Width;
        swapChainDesc.bVSync = true;
        swapChainDesc.NumBackBuffers = NUM_SWAPCHAIN_BACKBUFFER;

        if (!mpSwapChain->Create(swapChainDesc))
        {
            return false;
        }
    }

    return true;
}

void D3D12Backend::Shutdown()
{
    FlushGPU();

    mpSwapChain.reset();
    mpDevice.reset();
}

void D3D12Backend::OnResize(uint32_t Width, uint32_t Height)
{
    if (mpSwapChain)
    {
        mpSwapChain->Resize(Width, Height);
    }
}

void D3D12Backend::FlushGPU()
{
    if (mpDevice)
    {
        mpDevice->GetGraphicsCommandQueue()->Flush();
        mpDevice->GetComputeCommandQueue()->Flush();
        mpDevice->GetCopyCommandQueue()->Flush();
    }
}

void D3D12Backend::BeginFrame()
{
    if (mpDevice && mpDevice->GetBindlessDescriptorHeap())
    {
        mpDevice->GetBindlessDescriptorHeap()->ReleaseStaleSlots();
    }
}

void D3D12Backend::EndFrameAndPresent()
{
    mpSwapChain->Present();
    mpSwapChain->MoveToNextFrame();
}

FCommandContext* D3D12Backend::AllocateContext()
{
    return mpDevice->GetGraphicsCommandQueue()->AllocateContext();
}

uint64_t D3D12Backend::ExecuteGraphicsContext(FCommandContext* pCommandContext)
{
    return mpDevice->GetGraphicsCommandQueue()->ExecuteCommandContext(pCommandContext);
}