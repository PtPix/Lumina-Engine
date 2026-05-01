#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <vector>

#include "FCommandQueue.h"
#include "Renderer/D3D12Core/Resource/FTexture.h"

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

    bool Create(const FSwapChainCreateDesc& Desc);
    void Destroy();
    HRESULT Resize(int Width, int Height, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);

    HRESULT Present();
    void MoveToNextFrame();
    void WaitForGPU();

    // Getter
    [[nodiscard]] unsigned short GetNumBackBuffers() const { return mNumBackBuffers; }
    [[nodiscard]] unsigned short GetCurrentBackBufferIndex() const { return mCurrentBackBufferIndex; }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTVHandle() const { return mRenderTargets[mCurrentBackBufferIndex].GetRTV(); }
    [[nodiscard]] FTexture* GetCurrentRenderTargetResource() { return &mRenderTargets[mCurrentBackBufferIndex]; }

private:
    void CreateRenderTargetViews();
    void DestroyRenderTargetViews();

private:
    // Possess Resource
    Microsoft::WRL::ComPtr<IDXGISwapChain4> mpSwapChain;

    HWND mHwnd = nullptr;
    unsigned short mNumBackBuffers = 0;
    unsigned short mCurrentBackBufferIndex = 0;
    unsigned long long mNumTotalFrames = 0;
    bool mbVSync = false;

    // Observer
    FDevice* mpDevice = nullptr;
    FCommandQueue* mpPresentQueue = nullptr;

    std::vector<FTexture> mRenderTargets;
    std::vector<UINT64> mFenceValues;
    DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
};
