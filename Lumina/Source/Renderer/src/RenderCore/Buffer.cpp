#include <cassert>

#include "Renderer/D3D12Core/Common.h"
#include "Renderer/RenderCore/Buffer.h"

size_t StaticBufferHeap::MEMORY_ALIGNMENT = 256;

static D3D12_RESOURCE_STATES GetResourceTransitionState(EBufferType eType)
{
    D3D12_RESOURCE_STATES s = D3D12_RESOURCE_STATE_COMMON;
    switch (eType)
    {
    case CONSTANT_BUFFER : s = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
    case VERTEX_BUFFER   : s = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
    case INDEX_BUFFER    : s = D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
    default:
        LUMINA_LOG_WARNING(RHI, "StaticBufferPool::Create(): unkown resource type, couldn't determine resource transition state for upload.");
        break;
    }
    return s;
}

void StaticBufferHeap::Create(D3D12MA::Allocator* pAllocator, EBufferType Type, uint32_t TotalMemorySize, bool bUseVidMem,
                              const char* Name)
{
    mpAllocator = pAllocator;
    mTotalMemorySize = TotalMemorySize;
    mMemoryOffset = 0;
    mMemoryInit = 0;
    mpData = nullptr;
    mbUseVidMem = bUseVidMem;
    mBufferType = Type;

    D3D12_RESOURCE_DESC ResourceDesc = {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Width = TotalMemorySize;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT HResult = {};

    // Create Video Memory
    if (bUseVidMem)
    {
        D3D12MA::ALLOCATION_DESC AllocationDesc = {};
        AllocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        HResult = mpAllocator->CreateResource(
            &AllocationDesc, &ResourceDesc,
            GetResourceTransitionState(mBufferType),
            nullptr, &mpVidMemoryAllocation,
            IID_PPV_ARGS(&mpVidMemoryBuffer)
            );
        assert(SUCCEEDED(HResult));
        SetName(mpVidMemoryBuffer, Name);
    }

    // Create Upload Memory
    D3D12MA::ALLOCATION_DESC UploadAllocationDesc = {};
    UploadAllocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    HResult = mpAllocator->CreateResource(
        &UploadAllocationDesc, &ResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        &mpSystemMemoryAllocation, IID_PPV_ARGS(&mpSystemMemoryBuffer)
        );
    assert(SUCCEEDED(HResult));

    std::string UploadHeapName = std::string(Name) + "Upload";
    SetName(mpSystemMemoryBuffer, UploadHeapName.c_str());
    mpSystemMemoryBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mpData));
}

void StaticBufferHeap::Destroy()
{
    if (mbUseVidMem)
    {
        if (mpVidMemoryBuffer)
        {
            mpVidMemoryBuffer->Release();
            mpVidMemoryBuffer = nullptr;
        }
        if (mpVidMemoryAllocation)
        {
            mpVidMemoryAllocation->Release();
            mpVidMemoryAllocation = nullptr;
        }
    }
    if (mpSystemMemoryBuffer && mpData)
    {
        mpSystemMemoryBuffer->Unmap(0, nullptr);
        mpData = nullptr;
    }
    if (mpSystemMemoryBuffer)
    {
        mpSystemMemoryBuffer->Release();
        mpSystemMemoryBuffer = nullptr;
    }
    if (mpSystemMemoryAllocation)
    {
        mpSystemMemoryAllocation->Release();
        mpSystemMemoryAllocation = nullptr;
    }
}

bool StaticBufferHeap::AllocBuffer(uint32_t NumElements, uint32_t StrideInBytes, const void* pInitData,
    D3D12_GPU_VIRTUAL_ADDRESS* pBufferLocationOut, uint32_t* pSizeOut)
{
    void* pData = nullptr;
    if (AllocBuffer(NumElements, StrideInBytes, &pData, pBufferLocationOut, pSizeOut))
    {
        memcpy(pData, pInitData, static_cast<size_t>(NumElements) * StrideInBytes);
        return true;
    }
    return false;
}

bool StaticBufferHeap::AllocVertexBuffer(uint32_t NumVertices, uint32_t StrideInBytes, const void* pInitData,
    D3D12_VERTEX_BUFFER_VIEW* pViewOut)
{
    assert(mBufferType == EBufferType::VERTEX_BUFFER);
    void* pData = nullptr;
    if (AllocVertexBuffer(NumVertices, StrideInBytes, &pData, pViewOut))
    {
        memcpy(pData, pInitData, static_cast<size_t>(NumVertices) * StrideInBytes);
        return true;
    }
    return false;
}

bool StaticBufferHeap::AllocIndexBuffer(uint32_t NumIndices, uint32_t StrideInBytes, const void* pInitData,
    D3D12_INDEX_BUFFER_VIEW* pViewOut)
{
    assert(mBufferType == EBufferType::INDEX_BUFFER);
    void* pData = nullptr;
    if (AllocIndexBuffer(NumIndices, StrideInBytes, &pData, pViewOut))
    {
        memcpy(pData, pInitData, static_cast<size_t>(NumIndices) * StrideInBytes);
        return true;
    }
    return false;
}

bool StaticBufferHeap::AllocConstantBuffer(uint32_t NumElements, uint32_t StrideInBytes, void** ppMappedDataOut,
    D3D12_GPU_VIRTUAL_ADDRESS* pGpuAddressOut)
{
    assert(mBufferType == EBufferType::CONSTANT_BUFFER);
    uint32_t SizeOut = 0;
    return AllocBuffer(NumElements, StrideInBytes, ppMappedDataOut, pGpuAddressOut, &SizeOut);
}

void StaticBufferHeap::UploadData(ID3D12GraphicsCommandList* pCommandList)
{
    // Upload Heap could be read directly by GPU, so we only need to deal with video memory
    if (mbUseVidMem)
    {
        D3D12_RESOURCE_STATES ResourceState = GetResourceTransitionState(mBufferType);

        D3D12_RESOURCE_BARRIER BarrierToCopyDest = {};
        BarrierToCopyDest.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierToCopyDest.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        BarrierToCopyDest.Transition.pResource = mpVidMemoryBuffer;
        BarrierToCopyDest.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierToCopyDest.Transition.StateBefore = ResourceState;
        BarrierToCopyDest.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

        pCommandList->ResourceBarrier(1, &BarrierToCopyDest);
        pCommandList->CopyBufferRegion(mpVidMemoryBuffer, mMemoryInit, mpSystemMemoryBuffer, mMemoryInit, mMemoryOffset - mMemoryInit);

        D3D12_RESOURCE_BARRIER BarrierToState = {};
        BarrierToState.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierToState.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        BarrierToState.Transition.pResource = mpVidMemoryBuffer;
        BarrierToState.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierToState.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        BarrierToState.Transition.StateAfter = ResourceState;

        pCommandList->ResourceBarrier(1, &BarrierToState);

        mMemoryInit = mMemoryOffset;
    }
}

bool StaticBufferHeap::AllocBuffer(uint32_t NumElements, uint32_t StrideInBytes, void** ppDataOut,
    D3D12_GPU_VIRTUAL_ADDRESS* pBufferLocationOut, uint32_t* pSizeOut)
{
    uint32_t Size = AlignOffset(NumElements * StrideInBytes, (uint32_t)StaticBufferHeap::MEMORY_ALIGNMENT);
    const bool bHeapOutOfMemory = ((mMemoryOffset + Size) > mTotalMemorySize);
    if (bHeapOutOfMemory)
    {
        LUMINA_LOG_ERROR(RHI, "Static Heap out of Memory.");
        return false;
    }

    *ppDataOut = (void*)(mpData + mMemoryOffset);
    ID3D12Resource*& pResource = mbUseVidMem ? mpVidMemoryBuffer : mpSystemMemoryBuffer;
    *pBufferLocationOut = mMemoryOffset + pResource->GetGPUVirtualAddress();
    *pSizeOut = Size;
    mMemoryOffset += Size;

    return true;
}

bool StaticBufferHeap::AllocVertexBuffer(uint32_t NumVertices, uint32_t StrideInBytes, void** ppDataOut,
    D3D12_VERTEX_BUFFER_VIEW* pViewOut)
{
    bool bSuccess = AllocBuffer(NumVertices, StrideInBytes, ppDataOut, &pViewOut->BufferLocation, &pViewOut->SizeInBytes);
    pViewOut->StrideInBytes = bSuccess ? StrideInBytes : 0;
    return bSuccess;
}

bool StaticBufferHeap::AllocIndexBuffer(uint32_t NumIndices, uint32_t StrideInBytes, void** ppDataOut,
    D3D12_INDEX_BUFFER_VIEW* pViewOut)
{
    bool bSuccess = AllocBuffer(NumIndices, StrideInBytes, ppDataOut, &pViewOut->BufferLocation, &pViewOut->SizeInBytes);
    pViewOut->Format = bSuccess ? ((StrideInBytes == 4) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT) : DXGI_FORMAT_UNKNOWN;
    return bSuccess;
}

DynamicUploadHeap::~DynamicUploadHeap()
{
    Destroy();
}

bool DynamicUploadHeap::Initialize(D3D12MA::Allocator* pAllocator, uint32_t TotalSize)
{
    mTotalSize = TotalSize;
    mCurrentOffset = 0;

    D3D12_RESOURCE_DESC ResourceDesc = {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Width = mTotalSize;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC AllocationDesc = {};
    AllocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    HRESULT HResult = pAllocator->CreateResource(
        &AllocationDesc, &ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, &mpAllocation, IID_PPV_ARGS(&mpUploadBuffer)
        );

    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Failed to create DynamicUploadHeap via D3D12MA!");
        return false;
    }

    mpUploadBuffer->Map(0, nullptr, &mpCpuBaseAddress);
    mGpuBaseAddress = mpUploadBuffer->GetGPUVirtualAddress();

    return true;
}

void DynamicUploadHeap::Destroy()
{
    if (mpUploadBuffer)
    {
        mpUploadBuffer->Unmap(0, nullptr);
        mpUploadBuffer->Release();
        mpUploadBuffer = nullptr;
    }

    if (mpAllocation)
    {
        mpAllocation->Release();
        mpAllocation = nullptr;
    }

    mpCpuBaseAddress = nullptr;
    mGpuBaseAddress = 0;
    mCurrentOffset = 0;
}

FDynamicAllocation DynamicUploadHeap::Allocate(uint32_t SizeInBytes)
{
    uint32_t AlignedSize = (SizeInBytes + 255) & ~255;

    if (mCurrentOffset + AlignedSize > mTotalSize)
    {
        LUMINA_LOG_ERROR(RHI, "DynamicUploadHeap Out of Memory");
        return { nullptr, 0 };
    }

    FDynamicAllocation Allocation{};
    Allocation.CpuAddress = static_cast<uint8_t*>(mpCpuBaseAddress) + mCurrentOffset;
    Allocation.GpuAddress = mGpuBaseAddress + mCurrentOffset;

    mCurrentOffset += AlignedSize;

    return Allocation;
}

void DynamicUploadHeap::Reset()
{
    mCurrentOffset = 0;
}

