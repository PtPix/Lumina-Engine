/**
 * @file CommandQueue.h
 * @brief DirectX 12 CommandQueue Manage Class
 *
 * Wrap ID3D12CommandQueue create and life cycle.
 * Command Context allocate, execute and recycle.
 * Synchronize Signal, Wait, Flush
 */

#pragma once

#include "Renderer/D3D12Core/Core/CommandContext.h"

#include <d3d12.h>
#include <memory>
#include <queue>
#include <wrl/client.h>

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

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    void Create(FDevice* pDevice, ECommandQueueType Type,
        ECommandQueuePriority Priority = ECommandQueuePriority::NORMAL,
        const char* pName = nullptr);
    void Destroy();

    // ------------------------------------------------------------------------
    // Context Allocation & Execution
    // ------------------------------------------------------------------------
    FCommandContext* AllocateContext();
    uint64_t ExecuteCommandContext(FCommandContext* pContext);

    // ------------------------------------------------------------------------
    // Synchronization & Scheduling
    // ------------------------------------------------------------------------
    uint64_t Signal();
    bool IsFenceComplete(uint64_t FenceValue);
    void WaitForFenceValue(uint64_t FenceValue);
    void Flush();
    void WaitQueue(FCommandQueue* pOtherQueue, uint64_t FenceValue);

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    [[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const { return mpCommandQueue.Get(); }
    [[nodiscard]] uint64_t GetNextFenceValue() const { return mNextFenceValue; }
    [[nodiscard]] uint64_t GetLastCompletedFenceValue() const { return mLastCompletedFenceValue; }

private:
    void ReclaimContexts();
    D3D12_COMMAND_QUEUE_DESC CreateCommandQueueDesc(ECommandQueueType Type, ECommandQueuePriority Priority);

    // Core D3D12 Objects
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mpCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> mpFence;

    // Synchronization
    HANDLE mFenceEventHandle = nullptr;
    uint64_t mNextFenceValue = 1;
    uint64_t mLastCompletedFenceValue = 0;

    // Configuration
    FDevice* mpDevice = nullptr;
    ECommandQueueType mType;
    D3D12_COMMAND_LIST_TYPE mD3D12CommandListType;

    // Context Pool Management
    std::vector<std::unique_ptr<FCommandContext>> mContextPool;
    std::queue<FCommandContext*> mAvailableContexts;
    std::queue<std::pair<uint64_t, FCommandContext*>> mInFlightContexts;
};