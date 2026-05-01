#include <d3d12.h>

#include "Renderer/D3D12Core/Core/FCommandQueue.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Common.h"

void FCommandQueue::Create(FDevice* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority, const char* pName)
{
    if (!pDevice || !pDevice->GetDevice())
    {
        LUMINA_LOG_ERROR(RHI, "CommandQueue::Initialize failed: Null device");
        return;
    }

    mpDevice = pDevice;
    mType = Type;

    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = CreateCommandQueueDesc(Type, Priority);
    HRESULT HResult = pDevice->GetDevice()->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&this->mpCommandQueue));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Couldn't create Command Queue");
    }

    if (pName)
    {
        SetName(this->mpCommandQueue.Get(), pName);
    }

    // Create Execute fence
    pDevice->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence));
    mFenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    mNextFenceValue = 1;
    mLastCompletedFenceValue = 0;
}

void FCommandQueue::Destroy()
{
    if (mFenceEventHandle)
    {
        CloseHandle(mFenceEventHandle);
        mFenceEventHandle = nullptr;
    }
    mpFence.Reset();
    mpCommandQueue.Reset();
}

FCommandContext* FCommandQueue::AllocateContext()
{
    ReclaimContexts();

    if (mAvailableContexts.empty())
    {
        auto pNewContext = std::make_unique<FCommandContext>();
        pNewContext->Initialize(mpDevice, mD3D12CommandListType);
        pNewContext->Begin();

        mContextPool.push_back(std::move(pNewContext));
        return mContextPool.back().get();
    }

    FCommandContext* pContext = mAvailableContexts.front();
    mAvailableContexts.pop();

    pContext->Begin();
    return pContext;
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
        mpFence->SetEventOnCompletion(FenceValue, mFenceEventHandle);
        WaitForSingleObject(mFenceEventHandle, INFINITE);
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

uint64_t FCommandQueue::ExecuteCommandContext(FCommandContext* pContext)
{
    pContext->Close();

    ID3D12CommandList* ppCommandLists[] = { pContext->GetCommandList() };
    mpCommandQueue->ExecuteCommandLists(1, ppCommandLists);

    uint64_t FenceValue = Signal();
    mInFlightContexts.push({ FenceValue, pContext });

    return FenceValue;
}

void FCommandQueue::ReclaimContexts()
{
    while (!mInFlightContexts.empty())
    {
        auto& [FenceValue, pContext] = mInFlightContexts.front();

        if (IsFenceComplete(FenceValue))
        {
            mAvailableContexts.push(pContext);
            mInFlightContexts.pop();
        }
        else
        {
            break;
        }
    }
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
