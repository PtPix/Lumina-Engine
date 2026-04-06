#pragma once

#include <cstdint>
#include <d3d12.h>
#include <vector>
#include <wrl/client.h>

class ResourceView;

enum EResourceHeapType
{
    RTV_HEAP = 0,
    DSV_HEAP,
    CBV_SRV_UAV_HEAP,
    SAMPLER_HEAP,

    NUM_HEAP_TYPES
};

class StaticResourceViewHeap
{
public:
    void Create(ID3D12Device* pDevice, const wchar_t* ResourceName, EResourceHeapType HeapType, uint32_t Capacity, bool bCPUVisible = false);
    void Destroy();

    bool AllocateDescriptor(uint32_t Count, ResourceView* pResourceView);
    void FreeDescriptor(ResourceView* pResourceView);

    [[nodiscard]] ID3D12DescriptorHeap* GetHeap() const { return mpHeap; }

private:
    uint32_t mCapacity = 0;
    uint32_t mDescriptorElementSize = 0;
    std::vector<char> mIsDescriptorFree;

    ID3D12DescriptorHeap* mpHeap = nullptr;
    bool mbGPUVisible = false;
    EResourceHeapType mHeapType = {};
};

class UploadHeap
{
public:
    void Create(ID3D12Device* pDevice, size_t uSize, Microsoft::WRL::ComPtr<ID3D12CommandQueue> pQueue);
    void Destroy();

    uint8_t* SubAllocate(size_t uSize, uint64_t uAlign);

    [[nodiscard]] uint8_t* BasePtr() const { return mpDataBegin; }
    [[nodiscard]] ID3D12Resource* GetResource() const { return mpUploadHeap; }
    [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return mpCommandList; }
    [[nodiscard]] uint8_t* DataBegin() const { return mpDataBegin; }
    void UploadToGPUAndWait(Microsoft::WRL::ComPtr<ID3D12CommandQueue> pQueue = nullptr);

private:
    ID3D12Device* mpDevice = nullptr;
    ID3D12Resource* mpUploadHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mpCommandQueue;

    ID3D12GraphicsCommandList* mpCommandList = nullptr;
    ID3D12CommandAllocator* mpCommandAllocator = nullptr;

    uint8_t* mpDataCurrent = nullptr;
    uint8_t* mpDataEnd = nullptr;
    uint8_t* mpDataBegin = nullptr;

    ID3D12Fence* mpFence = nullptr;
    uint64_t mFenceValue = 0;
    HANDLE mHEvent = nullptr;
};