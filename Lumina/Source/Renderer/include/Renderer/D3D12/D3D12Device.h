/**
 * @file D3D12Device.h
 * @brief DirectX 12 Core Device Manage Class
 *
 * Wrap ID3D12Device and IDXGIAdapter's lifecycle.
 * Each FDevice Manages Graphic, Compute, Copy CommandQueue.
 * Init and possess D3D12 Memory Allocator.
 * Manage all kinds of CPU Descriptor Allocators and a GPU Bindless Descriptor Heap.
 * Offer query api on Hardware Capabilities.
 */

#pragma once

#include "D3D12MemAlloc.h"
#include "D3D12CommandQueue.h"
#include "D3D12DescriptorAllocator.h"
#include "D3D12BindlessDescriptorHeap.h"

#include <dxgiformat.h>
#include <memory>
#include <unordered_map>
#include <wrl/client.h>

struct IDXGIFactory6;
struct ID3D12Device;
struct IDXGIAdapter;

class FD3D12CommandQueue;
class FD3D12BindlessDescriptorHeap;

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

class FD3D12Device
{
public:
    FD3D12Device() = default;
    ~FD3D12Device() { Destroy(); }

    bool Create(const FDeviceCreateDesc& CreateDesc);
    void Destroy();

    // ------------------------------------------------------------------------
    // Core D3D12 Objects
    // ------------------------------------------------------------------------
    [[nodiscard]] ID3D12Device* GetDevice() const { return mpDevice.Get(); }
    [[nodiscard]] IDXGIAdapter* GetAdapter() const { return mpAdapter.Get(); }
    [[nodiscard]] D3D12MA::Allocator* GetAllocator() const { return mpAllocator.Get(); }

    // ------------------------------------------------------------------------
    // Command Queues
    // ------------------------------------------------------------------------
    [[nodiscard]] FD3D12CommandQueue* GetGraphicsCommandQueue() const { return mpGraphicsQueue.get(); }
    [[nodiscard]] FD3D12CommandQueue* GetComputeCommandQueue() const { return mpComputeQueue.get(); }
    [[nodiscard]] FD3D12CommandQueue* GetCopyCommandQueue() const { return mpCopyQueue.get(); }

    // ------------------------------------------------------------------------
    // Descriptor Management (CPU & GPU)
    // ------------------------------------------------------------------------
    [[nodiscard]] FD3D12DescriptorAllocator* GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) const { return mpDescriptorAllocators[Type].get(); }
    [[nodiscard]] FD3D12DescriptorAllocator* GetSRVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FD3D12DescriptorAllocator* GetUAVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FD3D12DescriptorAllocator* GetCBVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FD3D12DescriptorAllocator* GetRTVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].get(); }
    [[nodiscard]] FD3D12DescriptorAllocator* GetDSVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].get(); }

    [[nodiscard]] FD3D12BindlessDescriptorHeap* GetBindlessDescriptorHeap() const { return mpBindlessHeap.get(); }

    // ------------------------------------------------------------------------
    // Device Capabilities & Memory Query
    // ------------------------------------------------------------------------
    [[nodiscard]] unsigned GetDeviceMemoryMax() const;
    [[nodiscard]] unsigned GetDeviceMemoryAvailable() const;
    [[nodiscard]] const FDeviceCapabilities& GetDeviceCapabilities() const { return mCapabilities; }

private:
    Microsoft::WRL::ComPtr<ID3D12Device> mpDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> mpAdapter;
    Microsoft::WRL::ComPtr<D3D12MA::Allocator> mpAllocator;

    std::unique_ptr<FD3D12CommandQueue> mpGraphicsQueue = std::make_unique<FD3D12CommandQueue>();
    std::unique_ptr<FD3D12CommandQueue> mpComputeQueue = std::make_unique<FD3D12CommandQueue>();
    std::unique_ptr<FD3D12CommandQueue> mpCopyQueue = std::make_unique<FD3D12CommandQueue>();

    std::unique_ptr<FD3D12DescriptorAllocator> mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::unique_ptr<FD3D12BindlessDescriptorHeap> mpBindlessHeap = std::make_unique<FD3D12BindlessDescriptorHeap>();

    FDeviceCapabilities mCapabilities;
};

