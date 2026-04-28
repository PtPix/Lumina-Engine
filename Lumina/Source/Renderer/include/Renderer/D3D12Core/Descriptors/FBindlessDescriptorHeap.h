#pragma once

#include <cstdint>
#include <d3d12.h>
#include <mutex>
#include <queue>
#include <wrl/client.h>

class FDevice;

class FBindlessDescriptorHeap
{
public:
    FBindlessDescriptorHeap() = default;
    ~FBindlessDescriptorHeap();

    bool Initialize(FDevice* pDevice, UINT MaxDescriptors = 100000);

    uint32_t AllocateSlot();
    void FreeSlot(uint32_t Index);

    void CreateSRVFromCPUHandle(FDevice* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcCpuHandle, uint32_t DestIndex);

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32_t Index) const;
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint32_t Index) const;

    [[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeap() const { return mpDescriptorHeap.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mpDescriptorHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE mBaseCpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE mBaseGpuHandle = {};

    UINT mDescriptorSize = 0;
    UINT mMaxDescriptors = 0;

    std::mutex mAllocationMutex;
    UINT mCurrentWaterMark = 0;
    std::queue<uint32_t> mFreeSlots;
};