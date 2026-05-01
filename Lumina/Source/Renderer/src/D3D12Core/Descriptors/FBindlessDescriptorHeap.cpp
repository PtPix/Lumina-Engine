#include <cassert>

#include "Renderer/D3D12Core/Descriptors/FBindlessDescriptorHeap.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

FBindlessDescriptorHeap::~FBindlessDescriptorHeap()
{
    mpDescriptorHeap.Reset();
}

bool FBindlessDescriptorHeap::Initialize(const FDevice* pDevice, UINT MaxDescriptors)
{
    mMaxDescriptors = MaxDescriptors;
    mDescriptorSize = pDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
    HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    HeapDesc.NumDescriptors = mMaxDescriptors;
    HeapDesc.NodeMask = 0;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT HResult = pDevice->GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&mpDescriptorHeap));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "Failed to create bindless descriptor heap.");
        return false;
    }

    mBaseCpuHandle = mpDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    mBaseGpuHandle = mpDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    LUMINA_LOG_INFO(RHI, "Created Bindless descriptor Heap.");
    return true;
}

uint32_t FBindlessDescriptorHeap::AllocateSlot()
{
    std::lock_guard<std::mutex> Lock(mAllocationMutex);

    if (!mFreeSlots.empty())
    {
        uint32_t Index = mFreeSlots.front();
        mFreeSlots.pop();
        return Index;
    }

    assert(mCurrentWaterMark < mMaxDescriptors && "Bindless Descriptor Heap is Full!");
    uint32_t Index = mCurrentWaterMark;
    mCurrentWaterMark++;

    return Index;
}

void FBindlessDescriptorHeap::FreeSlot(uint32_t Index, FCommandQueue* pQueue, uint64_t FenceValue)
{
    std::lock_guard<std::mutex> Lock(mAllocationMutex);

    mDeferredFreeSlots.push_back( {pQueue, FenceValue, Index} );
}

void FBindlessDescriptorHeap::ReleaseStaleSlots()
{
    std::lock_guard<std::mutex> Lock(mAllocationMutex);

    for (size_t i = 0; i < mDeferredFreeSlots.size();)
    {
        if (mDeferredFreeSlots[i].pCommandQueue->IsFenceComplete(mDeferredFreeSlots[i].FenceValue))
        {
            mFreeSlots.push(mDeferredFreeSlots[i].SlotIndex);

            mDeferredFreeSlots[i] = mDeferredFreeSlots.back();
            mDeferredFreeSlots.pop_back();
        }
        else
        {
            i++;
        }
    }
}

void FBindlessDescriptorHeap::CreateSRVFromCPUHandle(const FDevice* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcCpuHandle,
                                                     uint32_t DestIndex) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE DestHandle = GetCpuHandle(DestIndex);
    pDevice->GetDevice()->CopyDescriptorsSimple(1, DestHandle, SrcCpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

D3D12_CPU_DESCRIPTOR_HANDLE FBindlessDescriptorHeap::GetCpuHandle(uint32_t Index) const
{
    return { mBaseCpuHandle.ptr + static_cast<SIZE_T>(Index) * mDescriptorSize };
}

D3D12_GPU_DESCRIPTOR_HANDLE FBindlessDescriptorHeap::GetGpuHandle(uint32_t Index) const
{
    return { mBaseGpuHandle.ptr + static_cast<SIZE_T>(Index) * mDescriptorSize };
}
