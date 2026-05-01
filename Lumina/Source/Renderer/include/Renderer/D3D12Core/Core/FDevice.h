// FDevice.h
// Create ID3D12Device, Manage CommandQueue, Allocate Cpu Descriptor, Manage Bindless Descriptor heap.

#pragma once

#include <dxgiformat.h>
#include <memory>
#include <unordered_map>
#include <wrl/client.h>

#include "D3D12MemAlloc.h"
#include "Renderer/D3D12Core/Core/FCommandQueue.h"
#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocator.h"
#include "Renderer/D3D12Core/Descriptors/FBindlessDescriptorHeap.h"

struct IDXGIFactory6;
struct ID3D12Device;
struct IDXGIAdapter;

class FCommandQueue;
class FBindlessDescriptorHeap;

struct FDeviceCreateDesc
{
    bool bEnableDebugLayer      = false;
    bool bEnableValidationLayer = false;
    IDXGIFactory6* pFactory     = nullptr;
};

struct FDeviceCapabilities
{
    bool bSupportsHardwareRayTracing = false;
    bool bSupportsWaveOptimization   = false;  // Wave Optimization
    bool bSupportsFP16               = false;  
    bool bSupportsMeshShaders        = false;  
    bool bSupportsSamplerFeedback    = false;  
    bool bSupportsTypedUAVLoads      = false;  
    bool bSupportsEnhancedBarriers   = false;  // Enhanced Barriers for Synchronization
    unsigned SupportedMaxMultiSampleQualityLevel = 0;

    std::unordered_map<DXGI_FORMAT, bool> TypedUAVLoadFormatSupportMap;
};

class FDevice
{
public:
    FDevice() = default;
    ~FDevice() { Destroy(); }

    bool Create(const FDeviceCreateDesc& CreateDesc);
    void Destroy();

    // Getters
    [[nodiscard]] ID3D12Device* GetDevice() const { return mpDevice.Get(); }
    [[nodiscard]] IDXGIAdapter* GetAdapter() const { return mpAdapter.Get(); }
    [[nodiscard]] D3D12MA::Allocator* GetAllocator() const { return mpAllocator.Get(); }

    // CommandQueue getter
    [[nodiscard]] FCommandQueue* GetGraphicsCommandQueue() const { return mpGraphicsQueue.get(); }
    [[nodiscard]] FCommandQueue* GetComputeCommandQueue() const { return mpComputeQueue.get(); }
    [[nodiscard]] FCommandQueue* GetCopyCommandQueue() const { return mpCopyQueue.get(); }

    // CPU Descriptor Allocators
    [[nodiscard]] FDescriptorAllocator* GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) const { return mpDescriptorAllocators[Type].get(); }
    [[nodiscard]] FDescriptorAllocator* GetSRVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetUAVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetCBVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetRTVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetDSVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].get(); }

    // Gpu Bindless Descriptor heap
    [[nodiscard]] FBindlessDescriptorHeap* GetBindlessDescriptorHeap() const { return mpBindlessHeap.get(); }

    // Device Information Query
    [[nodiscard]] unsigned GetDeviceMemoryMax() const;
    [[nodiscard]] unsigned GetDeviceMemoryAvailable() const;
    [[nodiscard]] const FDeviceCapabilities& GetDeviceCapabilities() const { return mCapabilities; }

private:
    // Basic Member
    Microsoft::WRL::ComPtr<ID3D12Device> mpDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> mpAdapter;
    Microsoft::WRL::ComPtr<D3D12MA::Allocator> mpAllocator;

    // Command Queue
    std::unique_ptr<FCommandQueue> mpGraphicsQueue = std::make_unique<FCommandQueue>();
    std::unique_ptr<FCommandQueue> mpComputeQueue = std::make_unique<FCommandQueue>();
    std::unique_ptr<FCommandQueue> mpCopyQueue = std::make_unique<FCommandQueue>();

    // Cpu Descriptor Allocators
    std::unique_ptr<FDescriptorAllocator> mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    // Gpu Bindless Descriptor Heap
    std::unique_ptr<FBindlessDescriptorHeap> mpBindlessHeap = std::make_unique<FBindlessDescriptorHeap>();

    // Hardware Information
    FDeviceCapabilities mCapabilities;
};

