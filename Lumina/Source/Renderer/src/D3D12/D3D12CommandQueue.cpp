#include <d3d12.h>

#include "Renderer/D3D12/D3D12CommandQueue.h"
#include "Renderer/D3D12/D3D12Device.h"
#include "Renderer/D3D12/D3D12Common.h"

void FD3D12CommandQueue::Create(FD3D12Device* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority, const char* pName)
{
    if (!pDevice || !pDevice->GetDevice())
    {
        LUMINA_LOG_ERROR(RHI, "CommandQueue::Initialize failed: Null device");
        return;
    }

    static constexpr D3D12_COMMAND_LIST_TYPE Types[] =
    {
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_TYPE_COMPUTE,
        D3D12_COMMAND_LIST_TYPE_COPY
    };

    mpDevice = pDevice;
    mType = Type;
    mD3D12CommandListType = Types[mType];

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

void FD3D12CommandQueue::Destroy()
{
    if (mFenceEventHandle)
    {
        CloseHandle(mFenceEventHandle);
        mFenceEventHandle = nullptr;
    }

    mpFence.Reset();
    mpCommandQueue.Reset();
}

FD3D12CommandContext* FD3D12CommandQueue::AllocateContext()
{
    ReclaimContexts();

    if (mAvailableContexts.empty())
    {
        auto pNewContext = std::make_unique<FD3D12CommandContext>();
        pNewContext->Initialize(mpDevice, mD3D12CommandListType);
        pNewContext->Begin();

        mContextPool.push_back(std::move(pNewContext));
        return mContextPool.back().get();
    }

    FD3D12CommandContext* pContext = mAvailableContexts.front();
    mAvailableContexts.pop();

    pContext->Begin();

    return pContext;
}

uint64_t FD3D12CommandQueue::ExecuteCommandContext(FD3D12CommandContext* pContext)
{
    pContext->Close();

    ID3D12CommandList* ppCommandLists[] = { pContext->GetCommandList() };
    mpCommandQueue->ExecuteCommandLists(1, ppCommandLists);

    uint64_t FenceValue = Signal();
    mInFlightContexts.push({ FenceValue, pContext });

    return FenceValue;
}

uint64_t FD3D12CommandQueue::Signal()
{
    uint64_t FenceValueToSignal = mNextFenceValue++;
    mpCommandQueue->Signal(mpFence.Get(), FenceValueToSignal);

    return FenceValueToSignal;
}

bool FD3D12CommandQueue::IsFenceComplete(uint64_t FenceValue)
{
    if (FenceValue > mLastCompletedFenceValue)
    {
        mLastCompletedFenceValue = max(mLastCompletedFenceValue, mpFence->GetCompletedValue());
    }

    return FenceValue <= mLastCompletedFenceValue;
}

void FD3D12CommandQueue::WaitForFenceValue(uint64_t FenceValue)
{
    if (!IsFenceComplete(FenceValue))
    {
        mpFence->SetEventOnCompletion(FenceValue, mFenceEventHandle);
        WaitForSingleObject(mFenceEventHandle, INFINITE);

        mLastCompletedFenceValue = FenceValue;
    }
}

void FD3D12CommandQueue::Flush()
{
    WaitForFenceValue(Signal());
}

void FD3D12CommandQueue::WaitQueue(FD3D12CommandQueue* pOtherQueue, uint64_t FenceValue)
{
    mpCommandQueue->Wait(pOtherQueue->mpFence.Get(), FenceValue);
}


void FD3D12CommandQueue::ReclaimContexts()
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
            // Context is still in flight; order is guaranteed, so we can break early
            break;
        }
    }
}

D3D12_COMMAND_QUEUE_DESC FD3D12CommandQueue::CreateCommandQueueDesc(ECommandQueueType Type, ECommandQueuePriority Priority)
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
