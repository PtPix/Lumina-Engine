#include <d3d12.h>

#include "Renderer/D3D12Core/Core/FCommandQueue.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Common.h"

void FCommandQueue::Create(ID3D12Device* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority, const char* pName)
{
    if (!pDevice)
    {
        LUMINA_LOG_ERROR(RHI, "CommandQueue::Initialize failed: Null device");
        return;
    }

    HRESULT HResult = {};
    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = CreateCommandQueueDesc(Type, Priority);

    HResult = pDevice->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&this->mpCommandQueue));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Couldn't create Command Queue");
    }

    if (pName)
    {
        SetName(this->mpCommandQueue.Get(), pName);
    }

    // Create Execute fence
    pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence));
    mFenceEvnetHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    mNextFenceValue = 1;
    mLastCompletedFenceValue = 0;
}

void FCommandQueue::Destroy()
{
    if (mFenceEvnetHandle)
    {
        CloseHandle(mFenceEvnetHandle);
        mFenceEvnetHandle = nullptr;
    }
    mpFence.Reset();
    mpCommandQueue.Reset();
}

uint64_t FCommandQueue::ExecuteCommandList(ID3D12CommandList* pCommandList)
{
    mpCommandQueue->ExecuteCommandLists(1, &pCommandList);
    return Signal();
}

uint64_t FCommandQueue::Signal()
{
    uint64_t FenceValueToSignal = mNextFenceValue++;
    mpCommandQueue->Signal(mpFence.Get(), FenceValueToSignal);
    return FenceValueToSignal;
}

bool FCommandQueue::IsFenceComplete(uint64_t FenceValue)
{
    if (FenceValue > mLastCompletedFenceValue)
    {
        mLastCompletedFenceValue = max(mLastCompletedFenceValue, mpFence->GetCompletedValue());
    }
    return FenceValue <= mLastCompletedFenceValue;
}

void FCommandQueue::WaitForFenceValue(uint64_t FenceValue)
{
    if (!IsFenceComplete(FenceValue))
    {
        mpFence->SetEventOnCompletion(FenceValue, mFenceEvnetHandle);
        WaitForSingleObject(mFenceEvnetHandle, INFINITE);
        mLastCompletedFenceValue = FenceValue;
    }
}

void FCommandQueue::Flush()
{
    WaitForFenceValue(Signal());
}

void FCommandQueue::WaitQueue(FCommandQueue* pOtherQueue, uint64_t FenceValue)
{
    mpCommandQueue->Wait(pOtherQueue->mpFence.Get(), FenceValue);
}

D3D12_COMMAND_QUEUE_DESC FCommandQueue::CreateCommandQueueDesc(ECommandQueueType Type, ECommandQueuePriority Priority)
{
    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
    CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    CommandQueueDesc.NodeMask = 0;

    static constexpr D3D12_COMMAND_QUEUE_PRIORITY Priorities[] =
    {
        D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
        D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME,
    };

    static constexpr D3D12_COMMAND_LIST_TYPE Types[] =
    {
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_TYPE_COMPUTE,
        D3D12_COMMAND_LIST_TYPE_COPY
    };

    CommandQueueDesc.Priority = Priorities[static_cast<size_t>(Priority)];
    CommandQueueDesc.Type = Types[static_cast<size_t>(Type)];

    return CommandQueueDesc;
}
