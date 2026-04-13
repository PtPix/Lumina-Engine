#include <cassert>

#include "Renderer/D3D12Core/Core/FSwapChain.h"
#include "Renderer/D3D12Core/Common.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

bool FSwapChain::Create(const FSwapChainCreateDesc& Desc)
{
    // Assertions and Assignment
    assert(Desc.pDevice);
    assert(Desc.pCommandQueue && Desc.pCommandQueue->GetCommandQueue());
    assert(Desc.NumBackBuffers > 0 && Desc.NumBackBuffers <= 3);

    this->mHwnd = Desc.Hwnd;
    this->mNumBackBuffers = Desc.NumBackBuffers;
    this->mpDevice = Desc.pDevice;
    this->mpPresentQueue = Desc.pCommandQueue;
    this->mbVSync = Desc.bVSync;
    this->mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Create DXGIFactory.
    HRESULT HResult = {};
    Microsoft::WRL::ComPtr<IDXGIFactory4> pDxgiFactory;
    HResult = CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "FSwapChain::Create(): Couldn't create DXGI Factory.");
        return false;
    }

    // Create Swapchain
    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
    SwapChainDesc.BufferCount = Desc.NumBackBuffers;
    SwapChainDesc.Height = Desc.WindowHeight;
    SwapChainDesc.Width = Desc.WindowWidth;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.Flags = Desc.bVSync ? 0 : DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain;
    HResult = pDxgiFactory->CreateSwapChainForHwnd(
        Desc.pCommandQueue->GetCommandQueue(),
        this->mHwnd,
        &SwapChainDesc,
        nullptr, nullptr,
        &pSwapChain
        );
    if (SUCCEEDED(HResult))
    {
        pSwapChain.As(&this->mpSwapChain);
    }
    else
    {
        LUMINA_LOG_ERROR(RHI, "FSwapChain::Create(): Couldn't create Swapchain.");
        return false;
    }

    this->mCurrentBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
    this->mFenceValues.resize(this->mNumBackBuffers, 0);

    // Create RTV for every buffer
    this->mRenderTargets.resize(this->mNumBackBuffers);
    CreateRenderTargetViews();

    return true;
}

void FSwapChain::Destroy()
{
    WaitForGPU();

    DestroyRenderTargetViews();
    mpSwapChain.Reset();
}

HRESULT FSwapChain::Resize(int Width, int Height, DXGI_FORMAT Format)
{
    DestroyRenderTargetViews();
    for (int i = 0; i < this->mNumBackBuffers; i++)
    {
        mFenceValues[i] = mFenceValues[mpSwapChain->GetCurrentBackBufferIndex()];
    }

    assert(this->mNumBackBuffers <= 3);
    IUnknown* const Buffers[3] = { mpPresentQueue->GetCommandQueue(), mpPresentQueue->GetCommandQueue(), mpPresentQueue->GetCommandQueue() };
    UINT NodeMasks[3] = { 1, 1, 1 };

    HRESULT HResult = mpSwapChain->ResizeBuffers1(
        (UINT)this->mNumBackBuffers,
        Width, Height,
        Format,
        this->mbVSync ? 0 : DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
        NodeMasks,
        Buffers
        );
    if (SUCCEEDED(HResult))
    {
        CreateRenderTargetViews();
    }

    this->mCurrentBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
    this->mFormat = Format;
    return HResult;
}

HRESULT FSwapChain::Present()
{
    constexpr UINT VSYNC_INTERVAL = 1;
    const bool& bVSync = this->mbVSync;

    // TODO: glitch should be dealed

    HRESULT HResult = {};
    UINT FlagPresent = bVSync ? 0 : DXGI_PRESENT_ALLOW_TEARING;
    if (bVSync)
    {
        HResult = mpSwapChain->Present(VSYNC_INTERVAL, FlagPresent);
    }
    else
    {
        HResult = mpSwapChain->Present(0, FlagPresent);
    }

    if (FAILED(HResult))
    {
        switch (HResult)
        {
        case DXGI_ERROR_DEVICE_RESET:
            LUMINA_LOG_ERROR(RHI, "FSwapChain::Present(): DXGI_ERROR_DEVICE_RESET");
            // TODO: call HandleDeviceReset() from whoever will be responsible
            break;
        case DXGI_ERROR_DEVICE_REMOVED:
            LUMINA_LOG_ERROR(RHI, "FSwapChain::Present(): DXGI_ERROR_DEVICE_REMOVED");
            break;
        case DXGI_ERROR_INVALID_CALL:
            LUMINA_LOG_ERROR(RHI, "FSwapChain::Present(): DXGI_ERROR_INVALID_CALL");
            // TODO:
            break;
        case DXGI_STATUS_OCCLUDED:
            LUMINA_LOG_WARNING(RHI, "FSwapChain::Present(): DXGI_STATUS_OCCLUDED");
            break;
        default:
            assert(false);
            break;
        }
    }
    return HResult;
}

void FSwapChain::MoveToNextFrame()
{
    mFenceValues[mCurrentBackBufferIndex] = mpPresentQueue->Signal();

    mCurrentBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
    ++mNumTotalFrames;

    mpPresentQueue->WaitForFenceValue(mFenceValues[mCurrentBackBufferIndex]);
}

void FSwapChain::WaitForGPU()
{
    if (!mpPresentQueue)
        return;

    mpPresentQueue->Flush();
}

void FSwapChain::CreateRenderTargetViews()
{
    HRESULT HResult = {};
    for (int i = 0; i < mNumBackBuffers; i++)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> pBackBuffer;
        HResult = mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(HResult))
        {
            assert(false);
        }
        mRenderTargets[i].CreateFromSwapChain(mpDevice, pBackBuffer.Get());
        SetName(this->mRenderTargets[i].GetResource(), "FSwapChain::RenderTarget[%d]", i);
    }
}

void FSwapChain::DestroyRenderTargetViews()
{
    mRenderTargets.clear();
}
