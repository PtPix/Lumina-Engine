/**
 * @file D3D12SwapChain.h
 * @brief DirectX 12 Swap Chain Manager.
 *
 * Handles presentation to a window surface using IDXGISwapChain4.
 * Manages back buffer rotation, vsync synchronization, and window resizing.
 */

#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>
#include <vector>

#include "D3D12Texture.h"

class FD3D12Device;
class FD3D12CommandQueue;

struct FSwapChainCreateDesc
{
    FD3D12Device* pDevice = nullptr;
    FD3D12CommandQueue* pCommandQueue = nullptr;

    HWND Hwnd = nullptr;
    uint32_t WindowWidth = 0;
    uint32_t WindowHeight = 0;
    int NumBackBuffers = 3;

    bool bVSync = false;
    bool bFullScreen = false;
    bool bHDR = false;
};

class FD3D12SwapChain
{
public:
    FD3D12SwapChain() = default;
    ~FD3D12SwapChain() { Destroy(); }

    FD3D12SwapChain(const FD3D12SwapChain&) = delete;
    FD3D12SwapChain& operator=(const FD3D12SwapChain&) = delete;

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
    [[nodiscard]] FD3D12Texture* GetCurrentRenderTargetResource() { return &mRenderTargets[mCurrentBackBufferIndex]; }

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
    FD3D12Device* mpDevice = nullptr;
    FD3D12CommandQueue* mpPresentQueue = nullptr;

    // Resources
    std::vector<FD3D12Texture> mRenderTargets;
    std::vector<UINT64> mFenceValues;

};
