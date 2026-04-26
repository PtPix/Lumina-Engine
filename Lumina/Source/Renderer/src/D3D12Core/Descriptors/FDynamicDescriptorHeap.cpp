#include <cassert>

#include "Renderer/D3D12Core/Descriptors/FDynamicDescriptorHeap.h"

#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

FDynamicDescriptorHeap::FDynamicDescriptorHeap(FDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE HeapType,
                                               UINT DescriptorsPerPage) : mpDevice(pDevice), mHeapType(HeapType), mDescriptorsPerPage(DescriptorsPerPage)
{
    mDescriptorSize = mpDevice->GetDevice()->GetDescriptorHandleIncrementSize(mHeapType);
}

FDynamicDescriptorHeap::~FDynamicDescriptorHeap()
{
    mAllocatedHeaps.clear();
    while (!mRetiredHeaps.empty()) mRetiredHeaps.pop();
    while (!mAvailableHeaps.empty()) mAvailableHeaps.pop();
}

void FDynamicDescriptorHeap::StageDescriptors(UINT RootParameterIndex, UINT Offset, UINT NumDescriptors,
    D3D12_CPU_DESCRIPTOR_HANDLE SrcHandle)
{
    assert(RootParameterIndex < MAX_ROOT_PARAMETERS);
    assert(Offset + NumDescriptors <= MAX_DESCRIPTORS_PER_TABLE);

    FDescriptorTableCache& Cache = mDescriptorTableCache[RootParameterIndex];

    Cache.NumDescriptors = max(Cache.NumDescriptors, Offset + NumDescriptors);

    for (UINT i = 0; i < NumDescriptors; ++i)
    {
        Cache.BaseCpuHandle[Offset + i].ptr = SrcHandle.ptr + i * mDescriptorSize;
    }

    mStaleDescriptorTableBitMask |= (1 << RootParameterIndex);
}

void FDynamicDescriptorHeap::CommitStagedDescriptorsForDraw(FCommandContext& Context)
{
    CommitDescriptorTables(Context, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}

void FDynamicDescriptorHeap::CommitStagedDescriptorsForDispatch(FCommandContext& Context)
{
    CommitDescriptorTables(Context, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}

void FDynamicDescriptorHeap::Reset()
{
    // Clear the CPU-side descriptor table cache so stale descriptors from a previous frame
    // don't get committed to the new frame's draw calls.
    for (UINT i = 0; i < MAX_ROOT_PARAMETERS; ++i)
    {
        mDescriptorTableCache[i].NumDescriptors = 0;
    }
    mStaleDescriptorTableBitMask = 0;
}

void FDynamicDescriptorHeap::CleanupUsedHeaps(uint64_t FenceValue)
{
    if (mCurrentHeapPtr)
    {
        mRetiredHeaps.push({ FenceValue, mCurrentHeapPtr });
        mCurrentHeapPtr = nullptr;
        mCurrentOffset = 0;
    }

    uint64_t SafeFenceValue = (FenceValue >= 2) ? (FenceValue - 2) : 0;
    while (!mRetiredHeaps.empty())
    {
        auto& RetiredHeap = mRetiredHeaps.front();
        if (RetiredHeap.first <= SafeFenceValue)
        {
            mAvailableHeaps.push(RetiredHeap.second);
            mRetiredHeaps.pop();
        }
        else
        {
            break;
        }
    }
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> FDynamicDescriptorHeap::RequestDescriptorHeap()
{
    if (!mAvailableHeaps.empty())
    {
        auto Heap = mAvailableHeaps.front();
        mAvailableHeaps.pop();
        return Heap;
    }

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
    HeapDesc.Type = mHeapType;
    HeapDesc.NumDescriptors = mDescriptorsPerPage;
    HeapDesc.NodeMask = 0;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> NewHeap;
    HRESULT HResult = mpDevice->GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&NewHeap));
    assert(SUCCEEDED(HResult));

    mAllocatedHeaps.push_back(NewHeap);
    return NewHeap;
}

void FDynamicDescriptorHeap::CommitDescriptorTables(FCommandContext& Context,
    void(ID3D12GraphicsCommandList::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
    if (mStaleDescriptorTableBitMask == 0) return;

    // Get how many descriptors needed to be allocated
    UINT NumDescriptorsToCommit = 0;
    DWORD RootIndex;
    unsigned long TempMask = mStaleDescriptorTableBitMask;

    while (_BitScanForward(&RootIndex, TempMask))
    {
        NumDescriptorsToCommit += mDescriptorTableCache[RootIndex].NumDescriptors;
        TempMask ^= (1 << RootIndex);
    }

    // check if GPU heap space is enough
    if (!mCurrentHeapPtr || mCurrentOffset + NumDescriptorsToCommit > mDescriptorsPerPage)
    {
        mCurrentHeapPtr = RequestDescriptorHeap();
        mCurrentOffset = 0;
        mCurrentCpuAddress = mCurrentHeapPtr->GetCPUDescriptorHandleForHeapStart();
        mCurrentGpuAddress = mCurrentHeapPtr->GetGPUDescriptorHandleForHeapStart();

        ID3D12DescriptorHeap* ppHeaps[] = { mCurrentHeapPtr.Get() };
        Context.GetCommandList()->SetDescriptorHeaps(1, ppHeaps);

        mStaleDescriptorTableBitMask = 0;
        for (UINT i = 0; i < MAX_ROOT_PARAMETERS; ++i)
        {
            if (mDescriptorTableCache[i].NumDescriptors > 0)
            {
                mStaleDescriptorTableBitMask |= (1 << i);
            }
        }
    }

    // Copy from CPU to GPU
    ID3D12Device* pDevice = mpDevice->GetDevice();
    TempMask = mStaleDescriptorTableBitMask;

    while (_BitScanForward(&RootIndex, TempMask))
    {
        UINT NumDescriptors = mDescriptorTableCache[RootIndex].NumDescriptors;
        D3D12_CPU_DESCRIPTOR_HANDLE DestHandle = { mCurrentCpuAddress.ptr + mCurrentOffset * mDescriptorSize };

        UINT DestRangeSize = NumDescriptors;
        D3D12_CPU_DESCRIPTOR_HANDLE SrcHandles[MAX_DESCRIPTORS_PER_TABLE];
        UINT SrcRangeSizes[MAX_DESCRIPTORS_PER_TABLE];

        for (UINT i = 0; i < NumDescriptors; ++i)
        {
            SrcHandles[i] = mDescriptorTableCache[RootIndex].BaseCpuHandle[i];
            SrcRangeSizes[i] = 1;
        }

        pDevice->CopyDescriptors(1, &DestHandle, &DestRangeSize,
            NumDescriptors, SrcHandles, SrcRangeSizes, mHeapType);

        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle = { mCurrentGpuAddress.ptr + mCurrentOffset * mDescriptorSize };
        (Context.GetCommandList()->*SetFunc)(RootIndex, GpuHandle);

        mCurrentOffset += NumDescriptors;
        TempMask ^= (1 << RootIndex);
    }

    mStaleDescriptorTableBitMask = 0;
}
