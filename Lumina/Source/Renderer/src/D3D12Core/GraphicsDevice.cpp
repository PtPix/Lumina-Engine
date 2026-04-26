#include <cassert>

#include "Renderer/D3D12Core/GraphicsDevice.h"

bool GraphicsDevice::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    this->mHwnd = Hwnd;

    // Create FDevice
    {
        LUMINA_TIME_LOG_SCOPE("Create FDevice");
        FDeviceCreateDesc DeviceCreateDesc = {true, true, nullptr};
        const bool bDeviceCreateSucceeded = mDevice.Create(DeviceCreateDesc);
        assert(bDeviceCreateSucceeded);
    }

    // Create Command Queues
    {
        LUMINA_TIME_LOG_SCOPE("Create Command Queues");
        mCommandQueues[GRAPHICS].Create(mDevice.GetDevice(), GRAPHICS);
        SetName(mCommandQueues[GRAPHICS].GetCommandQueue(), "Rendering Graphics Command Queue");
        mCommandQueues[COPY].Create(mDevice.GetDevice(), COPY);
        SetName(mCommandQueues[COPY].GetCommandQueue(), "Rendering Copy Command Queue");
        mCommandQueues[COMPUTE].Create(mDevice.GetDevice(), COMPUTE);
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
                mDevice.GetDevice()->CreateCommandAllocator(Type, IID_PPV_ARGS(&this->mpCommandAllocators[QueueType][BufferIndex]));
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
                mDevice.GetDevice()->CreateCommandList(0, Type,
                    mpCommandAllocators[QueueType][BufferIndex], nullptr,
                    IID_PPV_ARGS(&mpCommandLists[QueueType][BufferIndex]));
                static_cast<ID3D12GraphicsCommandList*>(mpCommandLists[QueueType][BufferIndex])->Close();
            }
        }
    }
    // {
    //     mGraphicsCommandContext[0].Initialize(&mDevice, GRAPHICS, mpCommandAllocators[GRAPHICS][0]);
    //     mGraphicsCommandContext[1].Initialize(&mDevice, GRAPHICS, mpCommandAllocators[GRAPHICS][1]);
    //     mGraphicsCommandContext[2].Initialize(&mDevice, GRAPHICS, mpCommandAllocators[GRAPHICS][2]);
    // }
    // Create D3D12MA
    {
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

    // Heaps Initialize
    {
        ID3D12Device* pDevice = mDevice.GetDevice();

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
            // ImGui 专用独立 shader-visible SRV heap（字体纹理 + 用户纹理）
            constexpr uint32_t NumImGuiSrvDesc = 64;
            mHeapImGuiSRV.Create(pDevice, L"HeapImGuiSRV", EResourceHeapType::CBV_SRV_UAV_HEAP, NumImGuiSrvDesc, false);
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

    // Create FSwapChain
    {
        FSwapChainCreateDesc swapChainDesc = {};
        swapChainDesc.pDevice = &mDevice;
        swapChainDesc.pCommandQueue = &mCommandQueues[GRAPHICS];
        swapChainDesc.Hwnd = Hwnd;
        swapChainDesc.WindowHeight = Height;
        swapChainDesc.WindowWidth = Width;
        swapChainDesc.bVSync = true;
        swapChainDesc.NumBackBuffers = NUM_SWAPCHAIN_BACKBUFFER;

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

    mCommandQueues[0].Destroy();
    mCommandQueues[1].Destroy();
    mCommandQueues[2].Destroy();
    mDevice.Destroy();
}

FCommandContext* GraphicsDevice::BeginFrame()
{
    mFrameIndex = mSwapChain.GetCurrentBackBufferIndex();
    FCommandContext* pCurrentContext = &mGraphicsCommandContext[mFrameIndex];

    pCurrentContext->Begin();

    // ID3D12DescriptorHeap* ppHeaps[] = { mHeapCBV_SRV_UAV.GetHeap() };
    // pCurrentContext->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCurrentContext->TransitionResource(mSwapChain.GetCurrentRenderTargetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pCurrentContext->FlushResourceBarriers();
    return pCurrentContext;
}

void GraphicsDevice::EndFrameAndPresent()
{
    FCommandContext* pCurrentContext = &mGraphicsCommandContext[mFrameIndex];

    pCurrentContext->TransitionResource(mSwapChain.GetCurrentRenderTargetResource(), D3D12_RESOURCE_STATE_PRESENT);
    pCurrentContext->Close();


    uint64_t FenceValue = mCommandQueues[GRAPHICS].ExecuteCommandList(pCurrentContext->GetCommandList());

    mSwapChain.Present();
    pCurrentContext->CleanupDynamicHeaps(FenceValue);
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
