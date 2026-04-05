#include <d3d12.h>

#include "Renderer/D3D12Core/Fence.h"
#include "Renderer/D3D12Core/Common.h"

void Fence::Create(ID3D12Device* pDevice, const char* pDebugName)
{
    mFenceValue = 0;
    pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence));
    SetName(mpFence, pDebugName);
    mHEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
}

void Fence::Destroy()
{
    if (mpFence)
    {
        mpFence->Release();
    }
    CloseHandle(mHEvent);
}

void Fence::Signal(ID3D12CommandQueue* pCommandQueue)
{
    ++mFenceValue;
    pCommandQueue->Signal(mpFence, mFenceValue);
}

void Fence::WaitOnCPU(uint64_t FenceWaitValue) const
{
    if (mpFence->GetCompletedValue() < FenceWaitValue)
    {
        mpFence->SetEventOnCompletion(FenceWaitValue, mHEvent);
        WaitForSingleObject(mHEvent, INFINITE);
    }
}

void Fence::WaitOnGPU(ID3D12CommandQueue* pCommandQueue)
{
    pCommandQueue->Wait(mpFence, mFenceValue);
}

void Fence::WaitOnGPU(ID3D12CommandQueue* pCommandQueue, uint64_t OlderFence)
{
    pCommandQueue->Wait(mpFence, OlderFence);
}
