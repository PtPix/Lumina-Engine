#include "Renderer/D3D12/D3D12BindlessDescriptorHeap.h"
#include "Renderer/D3D12/D3D12Device.h"
#include "Renderer/D3D12/D3D12CommandQueue.h"
#include "Renderer/D3D12/D3D12Common.h"

#include <cassert>

FD3D12BindlessDescriptorHeap::~FD3D12BindlessDescriptorHeap()
{
    mpDescriptorHeap.Reset();
}

bool FD3D12BindlessDescriptorHeap::Initialize(const FD3D12Device* pDevice, UINT MaxDescriptors)
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

uint32_t FD3D12BindlessDescriptorHeap::AllocateSlot()
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

void FD3D12BindlessDescriptorHeap::FreeSlot(uint32_t Index, FD3D12CommandQueue* pQueue, uint64_t FenceValue)
{
    if (Index == UINT32_MAX) return;

    std::lock_guard<std::mutex> Lock(mAllocationMutex);

    if (!pQueue)
    {
        mFreeSlots.push(Index);
        return;
    }
    mDeferredFreeSlots.push_back( {pQueue, FenceValue, Index} );
}

void FD3D12BindlessDescriptorHeap::ReleaseStaleSlots()
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

void FD3D12BindlessDescriptorHeap::CopyDescriptor(const FD3D12Device *pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcCpuHandle,
    uint32_t DestIndex) const
{
    if (SrcCpuHandle.ptr == 0) return;

    D3D12_CPU_DESCRIPTOR_HANDLE DestHandle = GetCpuHandle(DestIndex);
    pDevice->GetDevice()->CopyDescriptorsSimple(1, DestHandle, SrcCpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void FD3D12BindlessDescriptorHeap::CreateSRVFromCPUHandle(const FD3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcCpuHandle,
                                                     uint32_t DestIndex) const
{
    CopyDescriptor(pDevice, SrcCpuHandle, DestIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12BindlessDescriptorHeap::GetCpuHandle(uint32_t Index) const
{
    return { mBaseCpuHandle.ptr + static_cast<SIZE_T>(Index) * mDescriptorSize };
}

D3D12_GPU_DESCRIPTOR_HANDLE FD3D12BindlessDescriptorHeap::GetGpuHandle(uint32_t Index) const
{
    return { mBaseGpuHandle.ptr + static_cast<SIZE_T>(Index) * mDescriptorSize };
}
