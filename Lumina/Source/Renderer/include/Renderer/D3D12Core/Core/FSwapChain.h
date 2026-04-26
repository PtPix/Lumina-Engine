#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <vector>

#include "FCommandQueue.h"
#include "Renderer/D3D12Core/Resource/Texture.h"

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
    bool Create(const FSwapChainCreateDesc& Desc);
    void Destroy();
    HRESULT Resize(int Width, int Height, DXGI_FORMAT Format);

    HRESULT Present();
    void MoveToNextFrame();
    void WaitForGPU();

    // Getter
    [[nodiscard]] unsigned short GetNumBackBuffers() const { return mNumBackBuffers; }
    [[nodiscard]] unsigned short GetCurrentBackBufferIndex() const { return mCurrentBackBufferIndex; }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTVHandle() const { return mRenderTargets[mCurrentBackBufferIndex].GetRTV(); }
    [[nodiscard]] Texture* GetCurrentRenderTargetResource() { return &mRenderTargets[mCurrentBackBufferIndex]; }

private:
    void CreateRenderTargetViews();
    void DestroyRenderTargetViews();

private:
    HWND mHwnd = nullptr;
    unsigned short mNumBackBuffers = 0;
    unsigned short mCurrentBackBufferIndex = 0;
    unsigned long long mNumTotalFrames = 0;
    bool mbVSync = false;

    // Observer
    FDevice* mpDevice = nullptr;
    FCommandQueue* mpPresentQueue = nullptr;

    // Possess Resource
    Microsoft::WRL::ComPtr<IDXGISwapChain4> mpSwapChain;
    std::vector<UINT64> mFenceValues;

    std::vector<Texture> mRenderTargets;
    DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
};
