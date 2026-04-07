#include <cassert>

#include "Renderer/D3D12Core/Core/SwapChain.h"
#include "Renderer/D3D12Core/Common.h"

bool SwapChain::Create(const FSwapChainCreateDesc& Desc)
{
    // Assertions and Assignment
    assert(Desc.pDevice);
    assert(Desc.pCommandQueue && Desc.pCommandQueue->GetCommandQueue());
    assert(Desc.NumBackBuffers > 0 && Desc.NumBackBuffers <= 3);

    this->mHwnd = Desc.Hwnd;
    this->mNumBackBuffers = Desc.NumBackBuffers;
    this->mpDevice = Desc.pDevice;
    this->mpPresentQueue = Desc.pCommandQueue->GetCommandQueue();
    this->mbVSync = Desc.bVSync;
    this->mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Create DXGIFactory.
    HRESULT HResult = {};
    Microsoft::WRL::ComPtr<IDXGIFactory4> pDxgiFactory;
    HResult = CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "SwapChain::Create(): Couldn't create DXGI Factory.");
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
        std::string Reason;
        switch (HResult)
        {
        case E_OUTOFMEMORY: Reason = "Out of memory"; break;
        case DXGI_ERROR_INVALID_CALL: Reason = "Invalid call"; break;
        default: Reason = "Unknown error"; break;
        }
        Log::Error("Couldn't create Swapchain: %s", Reason);
    }

    // Create Fence and Fence Event
    this->mCurrentBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
    this->mFenceValues.resize(this->mNumBackBuffers, 0);
    D3D12_FENCE_FLAGS FenceFlags = D3D12_FENCE_FLAG_NONE;
    this->mpDevice->CreateFence(this->mFenceValues[this->mCurrentBackBufferIndex], FenceFlags, IID_PPV_ARGS(&this->mpFence));
    ++mFenceValues[this->mCurrentBackBufferIndex];
    this->mHEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    assert(this->mHEvent != nullptr);

    // Create Descriptor Heap for Render target views
    D3D12_DESCRIPTOR_HEAP_DESC RenderTargetViewHeapDesc = {};
    RenderTargetViewHeapDesc.NumDescriptors = this->mNumBackBuffers;
    RenderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RenderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HResult = mpDevice->CreateDescriptorHeap(&RenderTargetViewHeapDesc, IID_PPV_ARGS(&this->mpDescriptorHeapRTV));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI,"Swapchain::Create(): Couldn't create RTV Heap: %d", HResult);
        return false;
    }

    // Create RTV for every buffer
    this->mRenderTargets.resize(this->mNumBackBuffers);
    CreateRenderTargetViews();

    return true;
}

void SwapChain::Destroy()
{
    WaitForGPU();

    if (mHEvent)
    {
        CloseHandle(mHEvent);
        mHEvent = nullptr;
    }

    DestroyRenderTargetViews();
    mpDescriptorHeapRTV.Reset();
    mpSwapChain.Reset();
    mpFence.Reset();
}

HRESULT SwapChain::Resize(int Width, int Height, DXGI_FORMAT Format)
{
    DestroyRenderTargetViews();
    for (int i = 0; i < this->mNumBackBuffers; i++)
    {
        mFenceValues[i] = mFenceValues[mpSwapChain->GetCurrentBackBufferIndex()];
    }

    assert(this->mNumBackBuffers <= 3);
    IUnknown* const Buffers[3] = { mpPresentQueue, mpPresentQueue, mpPresentQueue };
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

HRESULT SwapChain::Present()
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
            LUMINA_LOG_ERROR(RHI, "SwapChain::Present(): DXGI_ERROR_DEVICE_RESET");
            // TODO: call HandleDeviceReset() from whoever will be responsible
            break;
        case DXGI_ERROR_DEVICE_REMOVED:
            LUMINA_LOG_ERROR(RHI, "SwapChain::Present(): DXGI_ERROR_DEVICE_REMOVED");
            break;
        case DXGI_ERROR_INVALID_CALL:
            LUMINA_LOG_ERROR(RHI, "SwapChain::Present(): DXGI_ERROR_INVALID_CALL");
            // TODO:
            break;
        case DXGI_STATUS_OCCLUDED:
            LUMINA_LOG_WARNING(RHI, "SwapChain::Present(): DXGI_STATUS_OCCLUDED");
            break;
        default:
            assert(false);
            break;
        }
    }
    return HResult;
}

void SwapChain::MoveToNextFrame()
{
    // Signal Command to queue
    const UINT64 CurrentFenceValue = mFenceValues[this->mCurrentBackBufferIndex];
    mpPresentQueue->Signal(mpFence.Get(), CurrentFenceValue);

    // Update FrameIndex
    mCurrentBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
    ++mNumTotalFrames;

    // Wait until the next frame is ok
    UINT64 FenceCompletedValue = mpFence->GetCompletedValue();
    HRESULT HResult = {};
    if (FenceCompletedValue < mFenceValues[mCurrentBackBufferIndex])
    {
        mpFence->SetEventOnCompletion(mFenceValues[mCurrentBackBufferIndex], mHEvent);
        HResult = WaitForSingleObjectEx(mHEvent, INFINITE, FALSE);
    }
    switch (HResult)
    {
    case S_OK: break;
    case WAIT_TIMEOUT:
        LUMINA_LOG_WARNING(
            RHI,
            "SwapChain<hwnd= %p> timed out on WaitForGPU(): Signal=%llu, ICurrBackBuffer=%d, NumFramesPresented=%llu"
            , mHwnd, mFenceValues[mCurrentBackBufferIndex], mCurrentBackBufferIndex, mNumTotalFrames);
        break;
    default: break;
    }

    mFenceValues[mCurrentBackBufferIndex] = CurrentFenceValue + 1;
}

void SwapChain::WaitForGPU()
{
    if (!mpPresentQueue || !mpFence || !mHEvent)
    {
        return;
    }

    const uint64_t FenceValueToSignal = mFenceValues[mCurrentBackBufferIndex];
    HRESULT HResult = mpPresentQueue->Signal(mpFence.Get(), FenceValueToSignal);
    if (FAILED(HResult))
    {
        return;
    }

    if (mpFence->GetCompletedValue() < FenceValueToSignal)
    {
        mpFence->SetEventOnCompletion(FenceValueToSignal, mHEvent);
        WaitForSingleObject(mHEvent, INFINITE);
    }

    mFenceValues[mCurrentBackBufferIndex]++;
}

void SwapChain::CreateRenderTargetViews()
{
    const UINT RenderTargetViewDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle{ mpDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart() };

    HRESULT HResult = {};
    for (int i = 0; i < mNumBackBuffers; i++)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> pBackBuffer;
        HResult = mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(HResult))
        {
            assert(false);
        }
        mRenderTargets[i].CreateFromSwapChain(pBackBuffer.Get());
        mpDevice->CreateRenderTargetView(mRenderTargets[i].GetResource(), nullptr, RTVHandle);
        RTVHandle.ptr += RenderTargetViewDescriptorSize;
        SetName(this->mRenderTargets[i].GetResource(), "SwapChain<hwnd= %p>::RenderTarget[%d]", this->mHwnd, i);
    }
}

void SwapChain::DestroyRenderTargetViews()
{
    mRenderTargets.clear();
}
