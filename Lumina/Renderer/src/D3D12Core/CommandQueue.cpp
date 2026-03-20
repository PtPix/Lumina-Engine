#include "../../include/Renderer/D3D12Core/CommandQueue.h"
#include "../../include/Renderer/D3D12Core/Device.h"
#include "../../include/Renderer/D3D12Core/Common.h"

#include "Logger/Logger.h"

#include <d3d12.h>

void CommandQueue::Create(Device* pDevice, ECommandQueueType Type, ECommandQueuePriority Priority, const char* pName)
{
    if (!pDevice)
    {
        Log::Error("CommandQueue::Initialize failed: Null device");
    }

    HRESULT HResult = {};
    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = CreateCommandQueueDesc(Type, Priority);
    ID3D12Device* pD3D12Device = pDevice->GetDevicePtr();

    HResult = pD3D12Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&this->mpCommandQueue));
    if (FAILED(HResult))
    {
        Log::Error("Couldn't create Command List: %s", "TODO:reason");
    }

    if (pName)
    {
        SetName(this->mpCommandQueue, pName);
    }
}

void CommandQueue::Destroy()
{
    if (mpCommandQueue)
    {
        mpCommandQueue->Release();
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
