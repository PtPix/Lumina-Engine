#include <cassert>

#include "Renderer/D3D12Core/D3D12Backend.h"

#include "Renderer/D3D12Core/Common.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Core/FSwapChain.h"
#include "Renderer/D3D12Core/Core/FCommandQueue.h"
#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocator.h"

FDevice D3D12Backend::mDevice;
FSwapChain D3D12Backend::mSwapChain;
D3D12MA::Allocator* D3D12Backend::mpAllocator;

FCommandQueue D3D12Backend::mGraphicsQueue;
FCommandQueue D3D12Backend::mComputeQueue;
FCommandQueue D3D12Backend::mCopyQueue;

FDescriptorAllocator D3D12Backend::mSrvUavCbvAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024);
FDescriptorAllocator D3D12Backend::mRtvAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64);
FDescriptorAllocator D3D12Backend::mDsvAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64);

FBindlessDescriptorHeap D3D12Backend::mBindlessDescriptorHeap;

std::vector<FCommandContext*> D3D12Backend::mContextPool[4];
std::vector<FCommandContext*> D3D12Backend::mAvailableContextPool[4];

bool D3D12Backend::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    // Create FDevice
    {
        LUMINA_TIME_LOG_SCOPE("Create FDevice");

        FDeviceCreateDesc DeviceCreateDesc = {true, false, nullptr};
        const bool bDeviceCreateSucceeded = mDevice.Create(DeviceCreateDesc);
        assert(bDeviceCreateSucceeded);
    }

    // Create Command Queues
    {
        LUMINA_TIME_LOG_SCOPE("Create Command Queues");

        mGraphicsQueue.Create(mDevice.GetDevice(), GRAPHICS);
        SetName(mGraphicsQueue.GetCommandQueue(), "Rendering Graphics Command Queue");
        mComputeQueue.Create(mDevice.GetDevice(), COMPUTE);
        SetName(mComputeQueue.GetCommandQueue(), "Rendering Compute Command Queue");
        mCopyQueue.Create(mDevice.GetDevice(), COPY);
        SetName(mCopyQueue.GetCommandQueue(), "Rendering Copy Command Queue");
    }

    // Create FSwapChain
    {
        LUMINA_TIME_LOG_SCOPE("Create Swap chain");

        FSwapChainCreateDesc swapChainDesc = {};
        swapChainDesc.pDevice = &mDevice;
        swapChainDesc.pCommandQueue = &mGraphicsQueue;
        swapChainDesc.Hwnd = Hwnd;
        swapChainDesc.WindowHeight = Height;
        swapChainDesc.WindowWidth = Width;
        swapChainDesc.bVSync = true;
        swapChainDesc.NumBackBuffers = NUM_SWAPCHAIN_BACKBUFFER;

        mSwapChain.Create(swapChainDesc);
    }

    // Create Descriptor Allocator
    {
        LUMINA_TIME_LOG_SCOPE("Create Descriptor Allocators");

        mSrvUavCbvAllocator.Initialize(&mDevice);
        mRtvAllocator.Initialize(&mDevice);
        mDsvAllocator.Initialize(&mDevice);
    }

    // Create D3D12MA
    {
        LUMINA_TIME_LOG_SCOPE("Create D3D12 Memory Allocator");
        D3D12MA::ALLOCATOR_DESC AllocatorDesc = {};
        AllocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
        AllocatorDesc.pDevice = mDevice.GetDevice();
        AllocatorDesc.pAdapter = mDevice.GetAdapter();

        HRESULT HResult = D3D12MA::CreateAllocator(&AllocatorDesc, &mpAllocator);
        if (FAILED(HResult))
        {
            LUMINA_LOG_ERROR(RHI, "D3D12MA allocator creation failed");
            assert(false);
        }
    }

    // Create Bindless Descriptor Heap
    {
        LUMINA_TIME_LOG_SCOPE("Create Bindless Descriptor Heap");
        mBindlessDescriptorHeap.Initialize(&mDevice);
    }

    return true;
}

void D3D12Backend::Shutdown()
{
    FlushGPU();

    for (int i = 0; i < 4; i++)
    {
        for (auto* Context : mContextPool[i])
        {
            delete Context;
        }
        mContextPool[i].clear();
        mAvailableContextPool[i].clear();
    }
}

void D3D12Backend::FlushGPU()
{
    mGraphicsQueue.Flush();
    mComputeQueue.Flush();
    mCopyQueue.Flush();
}

void D3D12Backend::BeginFrame()
{
}

void D3D12Backend::EndFrameAndPresent()
{
    mSwapChain.Present();
    mSwapChain.MoveToNextFrame();
}

static uint32_t GetQueueTypeIndex(D3D12_COMMAND_LIST_TYPE Type)
{
    switch (Type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:  return 0;
    case D3D12_COMMAND_LIST_TYPE_BUNDLE:  return 1;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE: return 2;
    case D3D12_COMMAND_LIST_TYPE_COPY:    return 3;
    default: assert(false && "Invalid Command List Type"); return 0;
    }
}

FCommandContext* D3D12Backend::AllocateContext(D3D12_COMMAND_LIST_TYPE Type)
{
    uint32_t Index = GetQueueTypeIndex(Type);
    FCommandContext* pContext = nullptr;

    {
        if (!mAvailableContextPool[Index].empty())
        {
            pContext = mAvailableContextPool[Index].back();
            mAvailableContextPool[Index].pop_back();
        }
        else
        {
            pContext = new FCommandContext();
            pContext->Initialize(&mDevice, Type);
            mContextPool[Index].push_back(pContext);
        }
    }

    pContext->Begin();

    return pContext;
}

void D3D12Backend::FreeContext(FCommandContext* pContext)
{
    assert(pContext != nullptr);
    uint32_t Index = GetQueueTypeIndex(pContext->GetType());
    mAvailableContextPool[Index].push_back(pContext);
}