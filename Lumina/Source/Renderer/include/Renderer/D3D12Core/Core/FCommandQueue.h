#pragma once
#include <d3d12.h>
#include <wrl/client.h>

struct ID3D12Device;
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

    void Create(ID3D12Device* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority = ECommandQueuePriority::NORMAL, const char* pName = nullptr);
    void Destroy();

    // Queue Schedule and Synchronize
    uint64_t ExecuteCommandList(ID3D12CommandList* pCommandList);
    uint64_t Signal();
    bool IsFenceComplete(uint64_t FenceValue);
    void WaitForFenceValue(uint64_t FenceValue);
    void Flush();
    void WaitQueue(FCommandQueue* pOtherQueue, uint64_t FenceValue);

    // Getter
    [[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const { return mpCommandQueue.Get(); }

private:
    static D3D12_COMMAND_QUEUE_DESC CreateCommandQueueDesc(ECommandQueueType Type, ECommandQueuePriority Priority);

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mpCommandQueue;

    Microsoft::WRL::ComPtr<ID3D12Fence> mpFence;
    uint64_t mNextFenceValue = 1;
    uint64_t mLastCompletedFenceValue = 0;
    HANDLE mFenceEvnetHandle = nullptr;
};