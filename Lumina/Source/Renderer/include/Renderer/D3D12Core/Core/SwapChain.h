/**
 * @file SwapChain.h
 * @brief DirectX 12 Swap Chain Manager.
 *
 * Handles presentation to a window surface using IDXGISwapChain4.
 * Manages back buffer rotation, vsync synchronization, and window resizing.
 */

#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>
#include <vector>

#include "Renderer/D3D12Core/Resource/Texture.h"

class FDevice;
class FCommandQueue;

struct FSwapChainCreateDesc
{
    FDevice* pDevice = nullptr;
    FCommandQueue* pCommandQueue = nullptr;

    HWND Hwnd = nullptr;
    uint32_t WindowWidth = 0;
    uint32_t WindowHeight = 0;
    int NumBackBuffers = 3;

    bool bVSync = false;
    bool bFullScreen = false;
    bool bHDR = false;
};

class FSwapChain
{
public:
    FSwapChain() = default;
    ~FSwapChain() { Destroy(); }

    FSwapChain(const FSwapChain&) = delete;
    FSwapChain& operator=(const FSwapChain&) = delete;

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    bool Create(const FSwapChainCreateDesc& Desc);
    void Destroy();
    HRESULT Resize(int Width, int Height, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);

    // ------------------------------------------------------------------------
    // Presentation & Frame Synchronization
    // ------------------------------------------------------------------------
    HRESULT Present();
    void MoveToNextFrame();
    void WaitForGPU();

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    [[nodiscard]] unsigned short GetNumBackBuffers() const { return mNumBackBuffers; }
    [[nodiscard]] unsigned short GetCurrentBackBufferIndex() const { return mCurrentBackBufferIndex; }

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTVHandle() const { return mRenderTargets[mCurrentBackBufferIndex].GetRTV(); }
    [[nodiscard]] FTexture* GetCurrentRenderTargetResource() { return &mRenderTargets[mCurrentBackBufferIndex]; }

private:
    void CreateRenderTargetViews();
    void DestroyRenderTargetViews();

    // Core DXGI Objects
    Microsoft::WRL::ComPtr<IDXGISwapChain4> mpSwapChain;
    HWND mHwnd = nullptr;

    // Frame and Buffer Tracking
    unsigned short mNumBackBuffers = 0;
    unsigned short mCurrentBackBufferIndex = 0;
    unsigned long long mNumTotalFrames = 0;

    // Configuration
    bool mbVSync = false;
    DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;

    // Core D3D12 References
    FDevice* mpDevice = nullptr;
    FCommandQueue* mpPresentQueue = nullptr;

    // Resources
    std::vector<FTexture> mRenderTargets;
    std::vector<UINT64> mFenceValues;

};
