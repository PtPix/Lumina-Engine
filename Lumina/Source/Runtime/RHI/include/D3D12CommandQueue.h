/**
 * @file D3D12CommandQueue.h
 * @brief DirectX 12 CommandQueue Manage Class
 *
 * Wrap ID3D12CommandQueue create and life cycle.
 * Command Context allocate, execute and recycle.
 * Synchronize Signal, Wait, Flush
 */

#pragma once

#include "D3D12CommandContext.h"

#include <d3d12.h>
#include <memory>
#include <queue>
#include <wrl/client.h>

class FD3D12Device;
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

class FD3D12CommandQueue
{
public:
    FD3D12CommandQueue() = default;
    ~FD3D12CommandQueue() { Destroy(); }

    FD3D12CommandQueue(const FD3D12CommandQueue&) = delete;
    FD3D12CommandQueue& operator=(const FD3D12CommandQueue&) = delete;

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    void Create(FD3D12Device* pDevice, ECommandQueueType Type,
        ECommandQueuePriority Priority = ECommandQueuePriority::NORMAL,
        const char* pName = nullptr);
    void Destroy();

    // ------------------------------------------------------------------------
    // Context Allocation & Execution
    // ------------------------------------------------------------------------
    FD3D12CommandContext* AllocateContext();
    uint64_t ExecuteCommandContext(FD3D12CommandContext* pContext);

    // ------------------------------------------------------------------------
    // Synchronization & Scheduling
    // ------------------------------------------------------------------------
    uint64_t Signal();
    bool IsFenceComplete(uint64_t FenceValue);
    void WaitForFenceValue(uint64_t FenceValue);
    void Flush();
    void WaitQueue(FD3D12CommandQueue* pOtherQueue, uint64_t FenceValue);

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
    FD3D12Device* mpDevice = nullptr;
    ECommandQueueType mType;
    D3D12_COMMAND_LIST_TYPE mD3D12CommandListType;

    // Context Pool Management
    std::vector<std::unique_ptr<FD3D12CommandContext>> mContextPool;
    std::queue<FD3D12CommandContext*> mAvailableContexts;
    std::queue<std::pair<uint64_t, FD3D12CommandContext*>> mInFlightContexts;
};