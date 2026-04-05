// Device.h
// Create and Destroy Device. Basic Queries.

#pragma once

#include <dxgiformat.h>
#include <unordered_map>

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

class Device
{
public:
    bool Create(const FDeviceCreateDesc& CreateDesc);
    void Destroy();
    [[nodiscard]] ID3D12Device* GetDevicePtr() const { return mpDevice; }
    [[nodiscard]] ID3D12Device4* GetDevice4Ptr() const { return mpDevice4; }
    [[nodiscard]] IDXGIAdapter* GetAdapterPtr() const { return mpAdapter; }

    [[nodiscard]] unsigned GetDeviceMemoryMax() const;
    [[nodiscard]] unsigned GetDeviceMemoryAvailable() const;

    [[nodiscard]] const FDeviceCapabilities& GetDeviceCapabilities() const { return mCapabilities; }

private:
    ID3D12Device* mpDevice   = nullptr;
    ID3D12Device4* mpDevice4 = nullptr;
    IDXGIAdapter* mpAdapter  = nullptr;

    // TODO: Multiple devices Support

    FDeviceCapabilities mCapabilities;
};

