#include <cassert>

#include "Renderer/D3D12Core/Resource/Buffer.h"
#include "Renderer/D3D12Core/Common.h"
#include "Logger/Logger.h"

bool Buffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t Alignment, D3D12_RESOURCE_FLAGS Flags,
                    D3D12_RESOURCE_STATES InitialState, D3D12_HEAP_TYPE HeapType, const wchar_t* pName)
{
    assert(pAllocator != nullptr);
    assert(SizeInBytes > 0);

    mBufferSize = AlignOffset(SizeInBytes, Alignment);
    mUsageState = InitialState;

    D3D12_RESOURCE_DESC ResourceDesc = {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Width = mBufferSize;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Flags = Flags;

    D3D12MA::ALLOCATION_DESC AllocationDesc = {};
    AllocationDesc.HeapType = HeapType;

    HRESULT HResult = pAllocator->CreateResource(
        &AllocationDesc,
        &ResourceDesc,
        mUsageState,
        nullptr,
        &mpAllocation,
        IID_PPV_ARGS(&mpResource)
        );

    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Failed to Allocate GpuBuffer.");
        return false;
    }
    if (pName)
    {
        mpResource->SetName(pName);
    }

    return true;
}

void Buffer::Destroy()
{
    mpAllocation.Reset();
    mpResource.Reset();
    mBufferSize = 0;
}

void* Buffer::Map()
{
    void* pMappedData = nullptr;
    if (mpResource)
    {
        D3D12_RANGE ReadRange = { 0, 0 };
        mpResource->Map(0, &ReadRange, &pMappedData);
    }
    return pMappedData;
}

void Buffer::Unmap()
{
    if (mpResource)
    {
        mpResource->Unmap(0, nullptr);
    }
}

bool VertexBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t StrideInBytes,
    const wchar_t* pName)
{
    if (!Buffer::Create(pAllocator, SizeInBytes, StrideInBytes, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
    {
        return false;
    }

    mView.BufferLocation = GetGPUVirtualAddress();
    mView.SizeInBytes = static_cast<UINT>(mBufferSize);
    mView.StrideInBytes = static_cast<UINT>(StrideInBytes);

    return true;
}

bool IndexBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, DXGI_FORMAT Format, const wchar_t* pName)
{
    if (!Buffer::Create(pAllocator, SizeInBytes, 16, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
    {
        return false;
    }

    mView.BufferLocation = GetGPUVirtualAddress();
    mView.SizeInBytes = static_cast<UINT>(mBufferSize);
    mView.Format = Format;

    return true;
}

bool ConstantBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName)
{
    size_t AlignedSize = CalcAlignedSize(SizeInBytes);

    if (!Buffer::Create(
        pAllocator, AlignedSize, 256, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, pName
        ))
    {
        return false;
    }

    return true;
}

bool UploadBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName)
{
    return Buffer::Create(
        pAllocator, SizeInBytes, 1,
        D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_HEAP_TYPE_UPLOAD, pName
        );
}
