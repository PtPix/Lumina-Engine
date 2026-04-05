#include <cassert>

#include "Renderer/D3D12Core/GraphicsDevice.h"

bool GraphicsDevice::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    this->mHwnd = Hwnd;

    // Create Device
    {
        FDeviceCreateDesc DeviceCreateDesc = {true, false, nullptr};
        const bool bDeviceCreateSucceeded = mDevice.Create(DeviceCreateDesc);
        assert(bDeviceCreateSucceeded);
    }

    // Create Command Queues
    {
        mCommandQueues[GRAPHICS].Create(&mDevice, GRAPHICS);
        SetName(mCommandQueues[GRAPHICS].GetCommandQueue(), "Rendering Graphics Command Queue");
        mCommandQueues[COPY].Create(&mDevice, COPY);
        SetName(mCommandQueues[COPY].GetCommandQueue(), "Rendering Copy Command Queue");
        mCommandQueues[COMPUTE].Create(&mDevice, COMPUTE);
        SetName(mCommandQueues[COMPUTE].GetCommandQueue(), "Rendering Compute Command Queue");
    }

    // Create Command Allocators
    {
        // Rendering Command Allocators
        for (int QueueType = 0; QueueType < NUM_COMMAND_QUEUE_TYPES; QueueType++)
        {
            D3D12_COMMAND_LIST_TYPE Type = GetDX12CommandListType(static_cast<ECommandQueueType>(QueueType));

            // Create for every swapchain buffer.
            auto fnGetCommandAllocatorName = [QueueType](int BufferIndex) -> std::string
            {
                std::string Name = "Rendering Command Allocator";
                switch (QueueType)
                {
                case GRAPHICS: Name += "Graphics"; break;
                case COPY: Name += "Copy"; break;
                case COMPUTE: Name += "Compute"; break;
                default: Name += "Unknown"; break;
                }
                Name += "[" + std::to_string(BufferIndex) + "]";
                return Name;
            };
            for (int BufferIndex = 0; BufferIndex < NUM_SWAPCHAIN_BACKBUFFER; BufferIndex++)
            {
                mDevice.GetDevicePtr()->CreateCommandAllocator(Type, IID_PPV_ARGS(&this->mpCommandAllocators[QueueType][BufferIndex]));
                SetName(mpCommandAllocators[QueueType][BufferIndex], fnGetCommandAllocatorName(BufferIndex).c_str());
            }
        }
    }

    // Create Command Lists
    {
        for (int QueueType = 0; QueueType < NUM_COMMAND_QUEUE_TYPES; QueueType++)
        {
            D3D12_COMMAND_LIST_TYPE Type = GetDX12CommandListType(static_cast<ECommandQueueType>(QueueType));
            for (int BufferIndex = 0; BufferIndex < NUM_SWAPCHAIN_BACKBUFFER; BufferIndex++)
            {
                mDevice.GetDevicePtr()->CreateCommandList(0, Type,
                    mpCommandAllocators[QueueType][BufferIndex], nullptr,
                    IID_PPV_ARGS(&mpCommandLists[QueueType][BufferIndex]));
                static_cast<ID3D12GraphicsCommandList*>(mpCommandLists[QueueType][BufferIndex])->Close();
            }
        }
    }

    // Create D3D12MA
    {
        D3D12MA::ALLOCATOR_DESC AllocatorDesc = {};
        AllocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
        AllocatorDesc.pDevice = mDevice.GetDevicePtr();
        AllocatorDesc.pAdapter = mDevice.GetAdapterPtr();

        HRESULT HResult = D3D12MA::CreateAllocator(&AllocatorDesc, &mpAllocator);
        if (FAILED(HResult))
        {
            Log::Error("D3D12MA allocator creation failed");
            assert(false);
        }
    }

    // Heaps Initialize
    {
        ID3D12Device* pDevice = mDevice.GetDevicePtr();

        // Static Upload Heap
        const uint32_t UPLOAD_HEAP_SIZE = (512 + 256 + 128) * 1024 * 1024;
        mHeapUpload.Create(pDevice, UPLOAD_HEAP_SIZE, mCommandQueues[GRAPHICS].GetCommandQueue());

        // Dynamic Upload Heap
        for (int i = 0; i < NUM_SWAPCHAIN_BACKBUFFER; i++)
        {
            mDynamicUploadHeaps[i].Initialize(mpAllocator, 1024 * 1024 * 8);
        }

        // Descriptor Heap create
        {
            constexpr uint32_t NumCbvDesc = 100;
            constexpr uint32_t NumSrvDesc = 8192;
            constexpr uint32_t NumUavDesc = 100;
            constexpr bool bCpuVisible = false;
            mHeapCBV_SRV_UAV.Create(pDevice, L"HeapCBV_SRV_UAV", EResourceHeapType::CBV_SRV_UAV_HEAP, NumCbvDesc + NumSrvDesc + NumUavDesc, bCpuVisible);
        }

        {
            constexpr uint32_t NumDsvDesc = 100;
            mHeapDSV.Create(pDevice, L"HeapDSV", EResourceHeapType::DSV_HEAP, NumDsvDesc);
        }

        {
            constexpr uint32_t NumRtvDesc = 1000;
            mHeapRTV.Create(pDevice, L"HeapRTV", EResourceHeapType::RTV_HEAP, NumRtvDesc);
        }

        // Static Buffer Create
        constexpr uint32_t STATIC_GEOMETRY_MEMORY_SIZE = 256 * 1024 * 1024;
        constexpr bool USE_GPU_MEMORY = true;
        {
            mStaticHeapVertexBuffer.Create(mpAllocator, EBufferType::VERTEX_BUFFER, STATIC_GEOMETRY_MEMORY_SIZE, USE_GPU_MEMORY, "LuminaRenderer::mStaticVertexBufferPool");
        }

        {
            mStaticHeapIndexBuffer.Create(mpAllocator, EBufferType::INDEX_BUFFER, STATIC_GEOMETRY_MEMORY_SIZE, USE_GPU_MEMORY, "LuminaRenderer::mStaticIndexBufferPool");
        }

        {
            mStaticHeapConstantBuffer.Create(mpAllocator, EBufferType::CONSTANT_BUFFER, STATIC_GEOMETRY_MEMORY_SIZE, false, "LuminaRenderer::mStaticConstantBufferPool");
        }
    }

    // Create SwapChain
    {
        FSwapChainCreateDesc swapChainDesc = {};
        swapChainDesc.pDevice = mDevice.GetDevicePtr();
        swapChainDesc.pCommandQueue = &mCommandQueues[GRAPHICS];
        swapChainDesc.Hwnd = Hwnd;
        swapChainDesc.WindowHeight = Height;
        swapChainDesc.WindowWidth = Width;
        swapChainDesc.bVSync = true;

        mSwapChain.Create(swapChainDesc);
    }

    return true;
}

void GraphicsDevice::Destroy()
{
    mSwapChain.WaitForGPU();

    for (auto& CommandLists : mpCommandLists)
    {
        for (auto& CommandList : CommandLists)
        {
            if (CommandList)
            {
                CommandList->Release();
            }
        }
    }

    for (auto& Allocators : mpCommandAllocators)
    {
        for (auto& Allocator : Allocators)
        {
            if (Allocator)
            {
                Allocator->Release();
            }
        }
    }

    mSwapChain.Destroy();

    for (auto& DynamicHeap : mDynamicUploadHeaps)
    {
        DynamicHeap.Destroy();
    }

    mStaticHeapVertexBuffer.Destroy();
    mStaticHeapIndexBuffer.Destroy();
    mStaticHeapConstantBuffer.Destroy();
    mHeapUpload.Destroy();

    mHeapRTV.Destroy();
    mHeapDSV.Destroy();
    mHeapCBV_SRV_UAV.Destroy();
    WCHAR* tempstr;
    mpAllocator->BuildStatsString(&tempstr, true);
    if (mpAllocator)
    {
        mpAllocator->Release();
        mpAllocator = nullptr;
    }

    for (int i = 0; i < NUM_COMMAND_QUEUE_TYPES; i++)
    {
        mCommandQueues[i].Destroy();
    }

    mDevice.Destroy();
}

ID3D12GraphicsCommandList* GraphicsDevice::BeginFrame()
{
    mFrameIndex = mSwapChain.GetCurrentBackBufferIndex();
    mpCommandAllocators[GRAPHICS][mFrameIndex]->Reset();
    auto* RenderCommandList = static_cast<ID3D12GraphicsCommandList*>(mpCommandLists[GRAPHICS][mFrameIndex]);
    RenderCommandList->Reset(mpCommandAllocators[GRAPHICS][mFrameIndex], nullptr);

    ID3D12DescriptorHeap* ppHeaps[] = { mHeapCBV_SRV_UAV.GetHeap() };
    RenderCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = mSwapChain.GetCurrentBackBufferRenderTarget();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    RenderCommandList->ResourceBarrier(1, &barrier);

    return RenderCommandList;
}

void GraphicsDevice::EndFrameAndPresent()
{
    auto* RenderCommandList = static_cast<ID3D12GraphicsCommandList*>(mpCommandLists[GRAPHICS][mFrameIndex]);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = mSwapChain.GetCurrentBackBufferRenderTarget();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    RenderCommandList->ResourceBarrier(1, &barrier);

    RenderCommandList->Close();
    ID3D12CommandList* ppCommandLists[] = { RenderCommandList };
    mCommandQueues[GRAPHICS].GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    mSwapChain.Present();
    mSwapChain.MoveToNextFrame();
}

void GraphicsDevice::OnResize(uint32_t Width, uint32_t Height)
{
}

D3D12_COMMAND_LIST_TYPE GraphicsDevice::GetDX12CommandListType(ECommandQueueType Type)
{
    static constexpr D3D12_COMMAND_LIST_TYPE Types[] =
    {
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_TYPE_COMPUTE,
        D3D12_COMMAND_LIST_TYPE_COPY
    };
    return Types[Type];
}
