// FCommandQueue.h
// Create CommandQueue, Execute, Manage Command Context.

#pragma once
#include <d3d12.h>
#include <memory>
#include <queue>
#include <wrl/client.h>

#include "FCommandContext.h"

class FDevice;
struct ID3D12CommandQueue;

enum ECommandQueueType
{
    GRAPHICS = 0,
    COMPUTE  = 1,
    COPY     = 2,

    NUM_COMMAND_QUEUE_TYPES
};

enum class ECommandQueuePriority
{
    NORMAL =   0,
    HIGH =     1,
    REALTIME = 2
};

class FCommandQueue
{
public:
    FCommandQueue() = default;
    ~FCommandQueue() { Destroy(); }
    FCommandQueue(const FCommandQueue&) = delete;
    FCommandQueue& operator=(const FCommandQueue&) = delete;

    void Create(FDevice* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority = ECommandQueuePriority::NORMAL, const char* pName = nullptr);
    void Destroy();

    // Context management
    FCommandContext* AllocateContext();
    uint64_t ExecuteCommandContext(FCommandContext* pContext);

    // Queue Schedule and Synchronize
    uint64_t Signal();
    bool IsFenceComplete(uint64_t FenceValue);
    void WaitForFenceValue(uint64_t FenceValue);
    void Flush();
    void WaitQueue(FCommandQueue* pOtherQueue, uint64_t FenceValue);

    // Getter
    [[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const { return mpCommandQueue.Get(); }
    [[nodiscard]] uint64_t GetNextFenceValue() const { return mNextFenceValue; }
    [[nodiscard]] uint64_t GetLastCompletedFenceValue() const { return mLastCompletedFenceValue; }

private:
    void ReclaimContexts();

    D3D12_COMMAND_QUEUE_DESC CreateCommandQueueDesc(ECommandQueueType Type, ECommandQueuePriority Priority);

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mpCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> mpFence;
    HANDLE mFenceEventHandle = nullptr;
    uint64_t mNextFenceValue = 1;
    uint64_t mLastCompletedFenceValue = 0;

    FDevice* mpDevice = nullptr;
    ECommandQueueType mType;
    D3D12_COMMAND_LIST_TYPE mD3D12CommandListType;

    std::vector<std::unique_ptr<FCommandContext>> mContextPool;
    std::queue<FCommandContext*> mAvailableContexts;
    std::queue<std::pair<uint64_t, FCommandContext*>> mInFlightContexts;
};