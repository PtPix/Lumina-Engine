#pragma once

#include <dxgiformat.h>
#include <unordered_map>
#include <wrl/client.h>

#include "D3D12MemAlloc.h"

struct IDXGIFactory6;
struct ID3D12Device4;
struct ID3D12Device;
struct IDXGIAdapter;

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
    bool Create(const FDeviceCreateDesc& CreateDesc);
    void Destroy();

    [[nodiscard]] ID3D12Device* GetDevice() const { return mpDevice.Get(); }
    [[nodiscard]] IDXGIAdapter* GetAdapter() const { return mpAdapter.Get(); }
    [[nodiscard]] D3D12MA::Allocator* GetAllocator() const { return mpAllocator.Get(); }

    [[nodiscard]] unsigned GetDeviceMemoryMax() const;
    [[nodiscard]] unsigned GetDeviceMemoryAvailable() const;

    [[nodiscard]] const FDeviceCapabilities& GetDeviceCapabilities() const { return mCapabilities; }

private:
    Microsoft::WRL::ComPtr<ID3D12Device> mpDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> mpAdapter;
    Microsoft::WRL::ComPtr<D3D12MA::Allocator> mpAllocator;

    FDeviceCapabilities mCapabilities;
};

