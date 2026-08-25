#include "Renderer/D3D12/D3D12Buffer.h"
#include "Renderer/D3D12/D3D12Common.h"
#include "Renderer/D3D12/D3D12Device.h"
#include "Renderer/D3D12/D3D12CommandQueue.h"
#include "Renderer/D3D12/D3D12DeferredReleaseQueue.h"
#include "Renderer/D3D12/D3D12DescriptorAllocator.h"
#include "Renderer/D3D12/D3D12BindlessDescriptorHeap.h"

#include <cassert>
#include <utility>

FD3D12Buffer::FD3D12Buffer(FD3D12Buffer&& Other) noexcept
{
    *this = std::move(Other);
}

FD3D12Buffer& FD3D12Buffer::operator=(FD3D12Buffer&& Other) noexcept
{
    if (this != &Other)
    {
        Destroy();

        mpAllocation = Other.mpAllocation;
        mBufferSize = Other.mBufferSize;
        mpResource = std::move(Other.mpResource);
        mAllSubresourcesState = Other.mAllSubresourcesState;
        mbAllSubresourcesSame = Other.mbAllSubresourcesSame;
        mSubresourceStates = std::move(Other.mSubresourceStates);
        mpPersistentMappedData = Other.mpPersistentMappedData;
        Other.mpPersistentMappedData = nullptr;

        Other.mpAllocation = nullptr;
    }
    return *this;
}

bool FD3D12Buffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t Alignment, D3D12_RESOURCE_FLAGS Flags,
                     D3D12_RESOURCE_STATES InitialState, D3D12_HEAP_TYPE HeapType, const wchar_t* pName)
{
    assert(pAllocator != nullptr);
    assert(SizeInBytes > 0);

    mBufferSize = AlignOffset(SizeInBytes, Alignment);
    InitStateTracking(1, InitialState);

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
        InitialState,
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

void FD3D12Buffer::Destroy()
{
    ReleasePersistentMapping();

    if (mpAllocation || mpResource)
    {
        auto pAllocation = mpAllocation;
        auto pResource   = mpResource;
        FD3D12DeferredReleaseQueue::Enqueue([pAllocation, pResource]() mutable
        {
            pAllocation.Reset();
            pResource.Reset();
        });

        mpAllocation.Reset();
        mpResource.Reset();
        mBufferSize = 0;
    }

    mpResource.Reset();
    mBufferSize = 0;
}

void* FD3D12Buffer::Map()
{
    void* pMappedData = nullptr;
    if (mpResource)
    {
        D3D12_RANGE ReadRange = { 0, 0 };
        mpResource->Map(0, &ReadRange, &pMappedData);
    }
    return pMappedData;
}

void FD3D12Buffer::Unmap()
{
    if (mpResource)
    {
        mpResource->Unmap(0, nullptr);
    }
}

void * FD3D12Buffer::GetPersistentMappedPtr()
{
    if (!mpPersistentMappedData && mpResource)
    {
        D3D12_RANGE ReadRange = { 0, 0 };
        mpResource->Map(0, &ReadRange, &mpPersistentMappedData);
    }
    return mpPersistentMappedData;
}

void FD3D12Buffer::ReleasePersistentMapping()
{
    if (mpPersistentMappedData && mpResource)
    {
        mpResource->Unmap(0, nullptr);
        mpPersistentMappedData = nullptr;
    }
}

bool FD3D12VertexBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t StrideInBytes,
                           const wchar_t* pName)
{
    if (!FD3D12Buffer::Create(pAllocator, SizeInBytes, StrideInBytes, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
    {
        return false;
    }

    mView.BufferLocation = GetGPUVirtualAddress();
    mView.SizeInBytes = static_cast<UINT>(mBufferSize);
    mView.StrideInBytes = static_cast<UINT>(StrideInBytes);

    return true;
}

bool FD3D12IndexBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, DXGI_FORMAT Format, const wchar_t* pName)
{
    if (!FD3D12Buffer::Create(pAllocator, SizeInBytes, 16, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
    {
        return false;
    }

    mView.BufferLocation = GetGPUVirtualAddress();
    mView.SizeInBytes = static_cast<UINT>(mBufferSize);
    mView.Format = Format;

    return true;
}

bool FD3D12ConstantBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName)
{
    size_t AlignedSize = CalcAlignedSize(SizeInBytes);

    if (!FD3D12Buffer::Create(
        pAllocator, AlignedSize, 256, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, pName
        ))
    {
        return false;
    }

    return true;
}

bool FD3D12UploadBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName)
{
    return FD3D12Buffer::Create(
        pAllocator, SizeInBytes, 256,
        D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_HEAP_TYPE_UPLOAD, pName
        );
}

bool FD3D12StructuredBuffer::Create(FD3D12Device *pDevice, D3D12MA::Allocator *pAllocator, uint32_t ElementSize,
    uint32_t ElementCount, bool bAllowUAV, bool bWithCounter, D3D12_HEAP_TYPE HeapType, const wchar_t *pName)
{
    assert(pDevice && pAllocator && ElementSize > 0 && ElementCount > 0);

    Destroy();

    mpDevice = pDevice;
    mElementSize = ElementSize;
    mElementCount = ElementCount;

    const uint64_t DataSize = static_cast<uint64_t>(ElementSize) * ElementCount;

    uint64_t TotalSize = DataSize;
    if (bWithCounter && bAllowUAV)
    {
        mCounterOffset = AlignOffset<uint64_t>(DataSize, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
        TotalSize = mCounterOffset + sizeof(uint32_t);
    }

    D3D12_RESOURCE_FLAGS Flags = bAllowUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    const D3D12_RESOURCE_STATES InitialState = (HeapType == D3D12_HEAP_TYPE_UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;

    if (!FD3D12Buffer::Create(pAllocator, TotalSize, 16, Flags, InitialState, HeapType, pName))
    {
        return false;
    }

    FD3D12BindlessDescriptorHeap* pHeap = pDevice->GetBindlessDescriptorHeap();

    // SRV
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format                     = DXGI_FORMAT_UNKNOWN;
        SRVDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        SRVDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SRVDesc.Buffer.FirstElement        = 0;
        SRVDesc.Buffer.NumElements         = ElementCount;
        SRVDesc.Buffer.StructureByteStride = ElementSize;
        SRVDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;

        mSRV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);
        pDevice->GetDevice()->CreateShaderResourceView(mpResource.Get(), &SRVDesc, mSRV.GetCpuHandle());

        if (pHeap)
        {
            mBindlessSRVIndex = pHeap->AllocateSlot();
            pHeap->CopyDescriptor(pDevice, mSRV.GetCpuHandle(), mBindlessSRVIndex);
        }
    }

    // UAV
    if (bAllowUAV)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.Format                      = DXGI_FORMAT_UNKNOWN;
        UAVDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        UAVDesc.Buffer.FirstElement         = 0;
        UAVDesc.Buffer.NumElements          = ElementCount;
        UAVDesc.Buffer.StructureByteStride  = ElementSize;
        UAVDesc.Buffer.CounterOffsetInBytes = bWithCounter ? mCounterOffset : 0;
        UAVDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

        ID3D12Resource* pCounterResource = bWithCounter ? mpResource.Get() : nullptr;

        mUAV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);
        pDevice->GetDevice()->CreateUnorderedAccessView(
            mpResource.Get(), pCounterResource, &UAVDesc, mUAV.GetCpuHandle());

        if (pHeap)
        {
            mBindlessUAVIndex = pHeap->AllocateSlot();
            pHeap->CopyDescriptor(pDevice, mUAV.GetCpuHandle(), mBindlessUAVIndex);
        }
    }

    return true;
}

void FD3D12StructuredBuffer::Destroy()
{
    if (mpDevice)
    {
        if (FD3D12BindlessDescriptorHeap* pHeap = mpDevice->GetBindlessDescriptorHeap())
        {
            FD3D12CommandQueue* pQueue = mpDevice->GetGraphicsCommandQueue();
            const uint64_t Fence = pQueue ? pQueue->GetNextFenceValue() : 0;

            if (mBindlessSRVIndex != InvalidBindlessIndex)
                pHeap->FreeSlot(mBindlessSRVIndex, pQueue, Fence);
            if (mBindlessUAVIndex != InvalidBindlessIndex)
                pHeap->FreeSlot(mBindlessUAVIndex, pQueue, Fence);
        }
    }
    mBindlessSRVIndex = InvalidBindlessIndex;
    mBindlessUAVIndex = InvalidBindlessIndex;

    mSRV.Free();
    mUAV.Free();

    mElementSize = 0;
    mElementCount = 0;
    mCounterOffset = 0;
    mpDevice = nullptr;

    FD3D12Buffer::Destroy();
}

bool FD3D12ByteAddressBuffer::Create(FD3D12Device *pDevice, D3D12MA::Allocator *pAllocator, size_t SizeInBytes, bool bAllowUAV,
    const wchar_t *pName)
{
    assert(pDevice && pAllocator && SizeInBytes > 0);

      Destroy();
      mpDevice = pDevice;

      const size_t AlignedSize = AlignOffset<size_t>(SizeInBytes, 4);

      D3D12_RESOURCE_FLAGS Flags = bAllowUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                                             : D3D12_RESOURCE_FLAG_NONE;

      if (!FD3D12Buffer::Create(pAllocator, AlignedSize, 16, Flags,
                           D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
      {
          return false;
      }

      FD3D12BindlessDescriptorHeap* pHeap = pDevice->GetBindlessDescriptorHeap();
      const UINT NumDWords = static_cast<UINT>(AlignedSize / 4);

      {
          D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
          SRVDesc.Format                  = DXGI_FORMAT_R32_TYPELESS;
          SRVDesc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
          SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
          SRVDesc.Buffer.FirstElement     = 0;
          SRVDesc.Buffer.NumElements      = NumDWords;
          SRVDesc.Buffer.Flags            = D3D12_BUFFER_SRV_FLAG_RAW;

          mSRV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);
          pDevice->GetDevice()->CreateShaderResourceView(mpResource.Get(), &SRVDesc, mSRV.GetCpuHandle());

          if (pHeap)
          {
              mBindlessSRVIndex = pHeap->AllocateSlot();
              pHeap->CopyDescriptor(pDevice, mSRV.GetCpuHandle(), mBindlessSRVIndex);
          }
      }

      if (bAllowUAV)
      {
          D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
          UAVDesc.Format              = DXGI_FORMAT_R32_TYPELESS;
          UAVDesc.ViewDimension       = D3D12_UAV_DIMENSION_BUFFER;
          UAVDesc.Buffer.FirstElement = 0;
          UAVDesc.Buffer.NumElements  = NumDWords;
          UAVDesc.Buffer.Flags        = D3D12_BUFFER_UAV_FLAG_RAW;

          mUAV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);
          pDevice->GetDevice()->CreateUnorderedAccessView(
              mpResource.Get(), nullptr, &UAVDesc, mUAV.GetCpuHandle());

          if (pHeap)
          {
              mBindlessUAVIndex = pHeap->AllocateSlot();
              pHeap->CopyDescriptor(pDevice, mUAV.GetCpuHandle(), mBindlessUAVIndex);
          }
      }

      return true;
}

void FD3D12ByteAddressBuffer::Destroy()
{
    if (mpDevice)
    {
        if (FD3D12BindlessDescriptorHeap* pHeap = mpDevice->GetBindlessDescriptorHeap())
        {
            FD3D12CommandQueue* pQueue = mpDevice->GetGraphicsCommandQueue();
            const uint64_t Fence  = pQueue ? pQueue->GetNextFenceValue() : 0;

            if (mBindlessSRVIndex != InvalidBindlessIndex) pHeap->FreeSlot(mBindlessSRVIndex, pQueue, Fence);
            if (mBindlessUAVIndex != InvalidBindlessIndex) pHeap->FreeSlot(mBindlessUAVIndex, pQueue, Fence);
        }
    }
    mBindlessSRVIndex = InvalidBindlessIndex;
    mBindlessUAVIndex = InvalidBindlessIndex;

    mSRV.Free();
    mUAV.Free();
    mpDevice = nullptr;

    FD3D12Buffer::Destroy();
}
