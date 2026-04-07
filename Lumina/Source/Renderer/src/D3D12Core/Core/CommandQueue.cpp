#include <d3d12.h>

#include "Renderer/D3D12Core/Core/CommandQueue.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Common.h"

void CommandQueue::Create(ID3D12Device* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority, const char* pName)
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
}

D3D12_COMMAND_QUEUE_DESC CommandQueue::CreateCommandQueueDesc(ECommandQueueType Type, ECommandQueuePriority Priority)
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
