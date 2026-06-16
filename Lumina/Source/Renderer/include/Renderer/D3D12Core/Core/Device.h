/**
 * @file Device.h
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
#include "Renderer/D3D12Core/Core/CommandQueue.h"
#include "Renderer/D3D12Core/Descriptors/DescriptorAllocator.h"
#include "Renderer/D3D12Core/Descriptors/BindlessDescriptorHeap.h"

#include <dxgiformat.h>
#include <memory>
#include <unordered_map>
#include <wrl/client.h>

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

    // ------------------------------------------------------------------------
    // Core D3D12 Objects
    // ------------------------------------------------------------------------
    [[nodiscard]] ID3D12Device* GetDevice() const { return mpDevice.Get(); }
    [[nodiscard]] IDXGIAdapter* GetAdapter() const { return mpAdapter.Get(); }
    [[nodiscard]] D3D12MA::Allocator* GetAllocator() const { return mpAllocator.Get(); }

    // ------------------------------------------------------------------------
    // Command Queues
    // ------------------------------------------------------------------------
    [[nodiscard]] FCommandQueue* GetGraphicsCommandQueue() const { return mpGraphicsQueue.get(); }
    [[nodiscard]] FCommandQueue* GetComputeCommandQueue() const { return mpComputeQueue.get(); }
    [[nodiscard]] FCommandQueue* GetCopyCommandQueue() const { return mpCopyQueue.get(); }

    // ------------------------------------------------------------------------
    // Descriptor Management (CPU & GPU)
    // ------------------------------------------------------------------------
    [[nodiscard]] FDescriptorAllocator* GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) const { return mpDescriptorAllocators[Type].get(); }
    [[nodiscard]] FDescriptorAllocator* GetSRVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetUAVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetCBVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetRTVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].get(); }
    [[nodiscard]] FDescriptorAllocator* GetDSVAllocator() const { return mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].get(); }

    [[nodiscard]] FBindlessDescriptorHeap* GetBindlessDescriptorHeap() const { return mpBindlessHeap.get(); }

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

    std::unique_ptr<FCommandQueue> mpGraphicsQueue = std::make_unique<FCommandQueue>();
    std::unique_ptr<FCommandQueue> mpComputeQueue = std::make_unique<FCommandQueue>();
    std::unique_ptr<FCommandQueue> mpCopyQueue = std::make_unique<FCommandQueue>();

    std::unique_ptr<FDescriptorAllocator> mpDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::unique_ptr<FBindlessDescriptorHeap> mpBindlessHeap = std::make_unique<FBindlessDescriptorHeap>();

    FDeviceCapabilities mCapabilities;
};

