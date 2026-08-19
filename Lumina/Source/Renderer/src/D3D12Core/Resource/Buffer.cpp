#include "Renderer/D3D12Core/Resource/Buffer.h"
#include "Renderer/D3D12Core/Common.h"
#include "Renderer/D3D12Core/Core/Device.h"
 #include "Renderer/D3D12Core/Core/CommandQueue.h"
 #include "Renderer/D3D12Core/Core/DeferredReleaseQueue.h"
 #include "Renderer/D3D12Core/Descriptors/DescriptorAllocator.h"
 #include "Renderer/D3D12Core/Descriptors/BindlessDescriptorHeap.h"

#include <cassert>
#include <utility>

#include "Renderer/Managers/TextureManager.h"

FBuffer::FBuffer(FBuffer&& Other) noexcept
{
    *this = std::move(Other);
}

FBuffer& FBuffer::operator=(FBuffer&& Other) noexcept
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

bool FBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t Alignment, D3D12_RESOURCE_FLAGS Flags,
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

void FBuffer::Destroy()
{
    ReleasePersistentMapping();

    if (mpAllocation || mpResource)
    {
        auto pAllocation = mpAllocation;
        auto pResource   = mpResource;
        FDeferredReleaseQueue::Enqueue([pAllocation, pResource]() mutable
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

void* FBuffer::Map()
{
    void* pMappedData = nullptr;
    if (mpResource)
    {
        D3D12_RANGE ReadRange = { 0, 0 };
        mpResource->Map(0, &ReadRange, &pMappedData);
    }
    return pMappedData;
}

void FBuffer::Unmap()
{
    if (mpResource)
    {
        mpResource->Unmap(0, nullptr);
    }
}

void * FBuffer::GetPersistentMappedPtr()
{
    if (!mpPersistentMappedData && mpResource)
    {
        D3D12_RANGE ReadRange = { 0, 0 };
        mpResource->Map(0, &ReadRange, &mpPersistentMappedData);
    }
    return mpPersistentMappedData;
}

void FBuffer::ReleasePersistentMapping()
{
    if (mpPersistentMappedData && mpResource)
    {
        mpResource->Unmap(0, nullptr);
        mpPersistentMappedData = nullptr;
    }
}

bool FVertexBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t StrideInBytes,
                           const wchar_t* pName)
{
    if (!FBuffer::Create(pAllocator, SizeInBytes, StrideInBytes, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
    {
        return false;
    }

    mView.BufferLocation = GetGPUVirtualAddress();
    mView.SizeInBytes = static_cast<UINT>(mBufferSize);
    mView.StrideInBytes = static_cast<UINT>(StrideInBytes);

    return true;
}

bool FIndexBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, DXGI_FORMAT Format, const wchar_t* pName)
{
    if (!FBuffer::Create(pAllocator, SizeInBytes, 16, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
    {
        return false;
    }

    mView.BufferLocation = GetGPUVirtualAddress();
    mView.SizeInBytes = static_cast<UINT>(mBufferSize);
    mView.Format = Format;

    return true;
}

bool FConstantBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName)
{
    size_t AlignedSize = CalcAlignedSize(SizeInBytes);

    if (!FBuffer::Create(
        pAllocator, AlignedSize, 256, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, pName
        ))
    {
        return false;
    }

    return true;
}

bool FUploadBuffer::Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName)
{
    return FBuffer::Create(
        pAllocator, SizeInBytes, 256,
        D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_HEAP_TYPE_UPLOAD, pName
        );
}

bool FStructuredBuffer::Create(FDevice *pDevice, D3D12MA::Allocator *pAllocator, uint32_t ElementSize,
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

    if (!FBuffer::Create(pAllocator, TotalSize, 16, Flags, InitialState, HeapType, pName))
    {
        return false;
    }

    FBindlessDescriptorHeap* pHeap = pDevice->GetBindlessDescriptorHeap();

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

void FStructuredBuffer::Destroy()
{
    if (mpDevice)
    {
        if (FBindlessDescriptorHeap* pHeap = mpDevice->GetBindlessDescriptorHeap())
        {
            FCommandQueue* pQueue = mpDevice->GetGraphicsCommandQueue();
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

    FBuffer::Destroy();
}

bool FByteAddressBuffer::Create(FDevice *pDevice, D3D12MA::Allocator *pAllocator, size_t SizeInBytes, bool bAllowUAV,
    const wchar_t *pName)
{
    assert(pDevice && pAllocator && SizeInBytes > 0);

      Destroy();
      mpDevice = pDevice;

      const size_t AlignedSize = AlignOffset<size_t>(SizeInBytes, 4);

      D3D12_RESOURCE_FLAGS Flags = bAllowUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                                             : D3D12_RESOURCE_FLAG_NONE;

      if (!FBuffer::Create(pAllocator, AlignedSize, 16, Flags,
                           D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, pName))
      {
          return false;
      }

      FBindlessDescriptorHeap* pHeap = pDevice->GetBindlessDescriptorHeap();
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

void FByteAddressBuffer::Destroy()
{
    if (mpDevice)
    {
        if (FBindlessDescriptorHeap* pHeap = mpDevice->GetBindlessDescriptorHeap())
        {
            FCommandQueue* pQueue = mpDevice->GetGraphicsCommandQueue();
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

    FBuffer::Destroy();
}
