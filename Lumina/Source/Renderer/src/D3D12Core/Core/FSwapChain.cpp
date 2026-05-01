#include <cassert>

#include "Renderer/D3D12Core/Core/FSwapChain.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Core/FCommandQueue.h"
#include "Renderer/D3D12Core/Common.h"

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

    mFenceValues.resize(mNumBackBuffers, 0);
    mRenderTargets.resize(mNumBackBuffers);

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
        mpPresentQueue->GetCommandQueue(),
        mHwnd,
        &SwapChainDesc,
        nullptr, nullptr,
        &pSwapChain
        );
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "FSwapChain::Create(): Couldn't create Swapchain.");
        return false;
    }

    // Ban ALT+ENTER
    pDxgiFactory->MakeWindowAssociation(mHwnd, DXGI_MWA_NO_ALT_ENTER);

    pSwapChain.As(&this->mpSwapChain);
    mCurrentBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

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
    if (!mpSwapChain) return S_FALSE;

    WaitForGPU();

    DestroyRenderTargetViews();

    HRESULT HResult = mpSwapChain->ResizeBuffers(
        mNumBackBuffers, Width, Height, Format,
        mbVSync ? 0 : DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
        );

    if (SUCCEEDED(HResult))
    {
        mCurrentBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
        mFormat = Format;
        CreateRenderTargetViews();
    }

    std::fill(mFenceValues.begin(), mFenceValues.end(), 0);

    return HResult;
}

HRESULT FSwapChain::Present()
{
    // TODO: glitch should be dealed

    UINT FlagPresent = mbVSync ? 0 : DXGI_PRESENT_ALLOW_TEARING;
    UINT SyncInterval = mbVSync ? 1 : 0;

    HRESULT HResult = mpSwapChain->Present(SyncInterval, FlagPresent);

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

    mpPresentQueue->WaitForFenceValue(mFenceValues[mCurrentBackBufferIndex]);

    ++mNumTotalFrames;
}

void FSwapChain::WaitForGPU()
{
    if (!mpPresentQueue)
        return;

    mpPresentQueue->Flush();
}

void FSwapChain::CreateRenderTargetViews()
{
    mRenderTargets.resize(mNumBackBuffers);

    for (int i = 0; i < mNumBackBuffers; i++)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> pBackBuffer;
        HRESULT HResult = mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
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
