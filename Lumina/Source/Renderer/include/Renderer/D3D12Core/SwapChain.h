#pragma once

#include "CommandQueue.h"

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <vector>

struct FSwapChainCreateDesc
{
    ID3D12Device* pDevice = nullptr;
    CommandQueue* pCommandQueue = nullptr;

    HWND Hwnd = nullptr;
    int WindowWidth = 0;
    int WindowHeight = 0;
    int NumBackBuffers = 2;

    bool bVSync = false;
    bool bFullScreen = false;
    bool bHDR = false;
};

class SwapChain
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
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTVHandle() const
    {
        const UINT RenderTargetViewDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        const D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle{ mpDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart().ptr + RenderTargetViewDescriptorSize * mCurrentBackBufferIndex };
        return RTVHandle;
    }
    [[nodiscard]] ID3D12Resource* GetCurrentBackBufferRenderTarget() const { return mRenderTargets[GetCurrentBackBufferIndex()]; }


private:
    void CreateRenderTargetViews();
    void DestroyRenderTargetViews();

private:
    HWND mHwnd = nullptr;
    unsigned short mNumBackBuffers = 0;
    unsigned short mCurrentBackBufferIndex = 0;
    unsigned long long mNumTotalFrames = 0;
    bool mbVSync = false;

    HANDLE mHEvent = nullptr;
    ID3D12Fence* mpFence = nullptr;
    std::vector<UINT64> mFenceValues;

    std::vector<ID3D12Resource*> mRenderTargets;
    ID3D12DescriptorHeap* mpDescriptorHeapRTV = nullptr;

    ID3D12Device* mpDevice = nullptr;
    IDXGIAdapter* mpAdapter = nullptr;

    IDXGISwapChain4* mpSwapChain = nullptr;
    ID3D12CommandQueue* mpPresentQueue = nullptr;
    DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
};
