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

class CommandQueue
{
public:
    void Create(ID3D12Device* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority = ECommandQueuePriority::NORMAL, const char* pName = nullptr);

    [[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const { return mpCommandQueue.Get(); }

private:
    static D3D12_COMMAND_QUEUE_DESC CreateCommandQueueDesc(ECommandQueueType Type, ECommandQueuePriority Priority);

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mpCommandQueue;
};