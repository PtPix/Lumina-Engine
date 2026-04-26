#pragma once

#include <cstdint>
#include <d3d12.h>
#include <queue>
#include <vector>
#include <wrl/client.h>

class FDevice;
class FCommandContext;

class FDynamicDescriptorHeap
{
public:
    FDynamicDescriptorHeap(FDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, UINT DescriptorsPerPage = 1024);
    ~FDynamicDescriptorHeap();

    void StageDescriptors(UINT RootParameterIndex, UINT Offset, UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE SrcHandle);

    void CommitStagedDescriptorsForDraw(FCommandContext& Context);
    void CommitStagedDescriptorsForDispatch(FCommandContext& Context);

    void CleanupUsedHeaps(uint64_t FenceValue);
    void Reset();

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();

    void CommitDescriptorTables(FCommandContext& Context, void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

private:
    FDevice* mpDevice = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
    UINT mDescriptorSize = 0;
    UINT mDescriptorsPerPage = 0;

    // CPU Descriptor Cache Pool
    static const UINT MAX_ROOT_PARAMETERS = 32;
    static const UINT MAX_DESCRIPTORS_PER_TABLE = 256;
    struct FDescriptorTableCache
    {
        UINT NumDescriptors = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE BaseCpuHandle[MAX_DESCRIPTORS_PER_TABLE];
    };

    FDescriptorTableCache mDescriptorTableCache[MAX_ROOT_PARAMETERS];

    uint32_t mStaleDescriptorTableBitMask = 0;
    uint32_t mStaleCBVBitMask = 0;

    // GPU Heap Manage
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCurrentHeapPtr;
    D3D12_CPU_DESCRIPTOR_HANDLE mCurrentCpuAddress;
    D3D12_GPU_DESCRIPTOR_HANDLE mCurrentGpuAddress;
    UINT mCurrentOffset = 0;

    std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> mAllocatedHeaps;
    std::queue<std::pair<uint64_t, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>> mRetiredHeaps;
    std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> mAvailableHeaps;
};