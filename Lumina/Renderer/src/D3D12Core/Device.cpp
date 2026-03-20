#include "../../include/Renderer/D3D12Core/Device.h"
#include "Logger/Logger.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

struct FGPUInfo
{
    std::string DeviceName;
    unsigned DeviceID;
    unsigned VendorID;
    size_t DedicatedGPUMemory;
    IDXGIAdapter1* pAdapter;
    D3D_FEATURE_LEVEL MaxSupportedFeatureLevel;

    [[nodiscard]] bool IsAMD() const { return (VendorID == 0x1002); }
    [[nodiscard]] bool IsNVIDIA() const { return (VendorID == 0x10DE); }
    [[nodiscard]] bool IsIntel() const { return (VendorID == 0x163C) || (VendorID == 0x8086) || (VendorID == 0x8087); }
};

/**
 * Check the device feature support for the given device.
 * @param pDevice The device to check the feature support for.
 * @param DeviceCapabilities The device capabilities to fill in.
 * @return None.
 * @note This function checks the support for the following features: Hardware ray tracing, Wave optimization, Half Precision Float, Mesh Shaders, Sampler Feedback, TypedUAVLoads and Enhanced Barrier.
 */
static void CheckDeviceFeautureSupport(ID3D12Device4* pDevice, FDeviceCapabilities& DeviceCapabilities)
{
    HRESULT HResult;

    // Check if hardware ray tracing is supported
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 FeatureOptions5 = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &FeatureOptions5, sizeof(FeatureOptions5));
        if (!SUCCEEDED(HResult))
        {
            Log::Warning("Device::CheckFeatureSupport() : Hardware ray tracing failed.");
        }
        else
        {
            DeviceCapabilities.bSupportsHardwareRayTracing = (FeatureOptions5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
        }
    }

    // Check if wave optimization is supported
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS1 FeatureOptions1 = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &FeatureOptions1, sizeof(FeatureOptions1));
        if (!SUCCEEDED(HResult))
        {
            Log::Warning("Device::CheckFeatureSupport() : Wave optimization failed.");
        }
        else
        {
            DeviceCapabilities.bSupportsWaveOptimization = FeatureOptions1.WaveOps;
        }
    }

    // Check if Half Precision Float is supported
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS4 FeatureOptions4 = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &FeatureOptions4, sizeof(FeatureOptions4));
        if (!SUCCEEDED(HResult))
        {
            Log::Warning("Device::CheckFeatureSupport() : Half Precision Float failed.");
        }
        else
        {
            DeviceCapabilities.bSupportsFP16 = FeatureOptions4.Native16BitShaderOpsSupported;
        }
    }

    // Check if Mesh Shaders and Sampler Feedback are supported
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 FeatureOptions7 = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &FeatureOptions7, sizeof(FeatureOptions7));
        if (!SUCCEEDED(HResult))
        {
            Log::Warning("Device::CheckFeatureSupport() : Mesh Shaders and Sampler Feedback failed.");
        }
        else
        {
            DeviceCapabilities.bSupportsMeshShaders = FeatureOptions7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
            DeviceCapabilities.bSupportsSamplerFeedback = FeatureOptions7.SamplerFeedbackTier != D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED;
        }
    }

    // Check if Enhanced Barrier is supported
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS12 FeatureOptions12 = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &FeatureOptions12, sizeof(FeatureOptions12));
        if (!SUCCEEDED(HResult))
        {
            // Log::Warning("Device::CheckFeatureSupport() : Enhanced Barrier failed.");
        }
        else
        {
            DeviceCapabilities.bSupportsEnhancedBarriers = FeatureOptions12.EnhancedBarriersSupported;
        }
    }

    // Check if TypedUAVLoads is supported
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureOptions0 = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureOptions0, sizeof(FeatureOptions0));
        
        // Here we should check if the feature is supported and other formats are supported.
        if (!SUCCEEDED(HResult))
        {
            Log::Warning("Device::CheckFeatureSupport() : TypedUAVLoads failed.");
        }
        else
        {
            DeviceCapabilities.bSupportsTypedUAVLoads = FeatureOptions0.TypedUAVLoadAdditionalFormats;
            
            // Check other formats.
            if (DeviceCapabilities.bSupportsTypedUAVLoads)
            {
                const std::vector<DXGI_FORMAT> FormatsToCheck =
                {
                      DXGI_FORMAT_R16G16B16A16_UNORM
                    , DXGI_FORMAT_R16G16B16A16_SNORM
                    , DXGI_FORMAT_R32G32_FLOAT
                    , DXGI_FORMAT_R32G32_UINT
                    , DXGI_FORMAT_R32G32_SINT
                    , DXGI_FORMAT_R10G10B10A2_UNORM
                    , DXGI_FORMAT_R10G10B10A2_UINT
                    , DXGI_FORMAT_R11G11B10_FLOAT
                    , DXGI_FORMAT_R8G8B8A8_SNORM
                    , DXGI_FORMAT_R16G16_FLOAT
                    , DXGI_FORMAT_R16G16_UNORM
                    , DXGI_FORMAT_R16G16_UINT
                    , DXGI_FORMAT_R16G16_SNORM
                    , DXGI_FORMAT_R16G16_SINT
                    , DXGI_FORMAT_R8G8_UNORM
                    , DXGI_FORMAT_R8G8_UINT
                    , DXGI_FORMAT_R8G8_SNORM
                    , DXGI_FORMAT_R8G8_SINT
                    , DXGI_FORMAT_R16_UNORM
                    , DXGI_FORMAT_R16_SNORM
                    , DXGI_FORMAT_R8_SNORM
                    , DXGI_FORMAT_A8_UNORM
                    , DXGI_FORMAT_B5G6R5_UNORM
                    , DXGI_FORMAT_B5G5R5A1_UNORM
                    , DXGI_FORMAT_B4G4R4A4_UNORM
                };

                for (DXGI_FORMAT Format : FormatsToCheck)
                {
                    D3D12_FEATURE_DATA_FORMAT_SUPPORT FeatureDataSupport0 = 
                    {
                        Format,
                        D3D12_FORMAT_SUPPORT1_NONE,
                        D3D12_FORMAT_SUPPORT2_NONE
                    };

                    HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &FeatureDataSupport0, sizeof(FeatureDataSupport0));
                    if (SUCCEEDED(HResult) && (FeatureDataSupport0.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
                    {
                        DeviceCapabilities.TypedUAVLoadFormatSupportMap[Format] = true;
                    }
                    else
                    {
                        // Log::Warning("Device::CheckFeatureSupport() : TypedUAVLoads for DXGI_FORMAT:%d failed.", Format);
                    }
                }
            }
        }
    }

    // Check Max MSAA Quality
    {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS FeatureMSAAQualityLevels = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &FeatureMSAAQualityLevels, sizeof(FeatureMSAAQualityLevels));
        if (!SUCCEEDED(HResult))
        {
            // Log::Warning("Device::CheckFeatureSupport() : MSAA Quality failed.");
        }
        else
        {
            DeviceCapabilities.SupportedMaxMultiSampleQualityLevel = FeatureMSAAQualityLevels.NumQualityLevels;
        }
    }
}

std::string WideToASCII(const wchar_t* wideStr) {
    if (wideStr == nullptr) return "";

    const int bufferSize = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0) return "";
    
    std::string asciiStr(bufferSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wideStr, -1, &asciiStr[0], bufferSize, nullptr, nullptr);
    
    asciiStr.resize(bufferSize - 1);
    return asciiStr;
}

/**
 * Enumerate all DX12 adapters on the system, and create a GPU info struct for each adapter.
 * @param bEnableDebugLayer If true, create the DXGIFactory with the debug flag set.
 * @param bEnumerateSoftwareAdapters If true, enumerate software adapters as well as hardware adapters.
 * @param pFactory If not nullptr, use this DXGIFactory instead of creating a new one.
 * @return A vector of GPU info structs, each containing information about a DX12 adapter.
 */
static std::vector<FGPUInfo> EnumerateDX12Adapters(bool bEnableDebugLayer, bool bEnumerateSoftwareAdapters, IDXGIFactory6* pFactory)
{
    std::vector<FGPUInfo> GPUs;
    HRESULT HResult = {};

    IDXGIAdapter1* pAdapter = nullptr; // Graphics Card
    int32_t iAdapter = 0;              // Start from index 0 
    bool bAdapterFound = false;        // Found one will be true

    // We need to use DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE here, so we need a DXGIFactory.
    IDXGIFactory6* pDXGIFactory = nullptr;
    if (pFactory)
    {
        pDXGIFactory = pFactory;
    }
    else
    {
        UINT DXGIFlags = 0;
        if (bEnableDebugLayer)
        {
            DXGIFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
        HResult = CreateDXGIFactory2(DXGIFlags, IID_PPV_ARGS(&pDXGIFactory));
    }

    // Lambda function for adding adapters to GPUs
    auto FunctionAddAdapter = [&bAdapterFound, &GPUs](IDXGIAdapter1*& pAdapter, const DXGI_ADAPTER_DESC1& Desc, D3D_FEATURE_LEVEL FeatureLevel)
    {
        bAdapterFound = true;

        FGPUInfo GPUInfo = {};
        GPUInfo.DedicatedGPUMemory = Desc.DedicatedVideoMemory;
        GPUInfo.DeviceID = Desc.DeviceId;
        GPUInfo.DeviceName = WideToASCII(Desc.Description);
        GPUInfo.MaxSupportedFeatureLevel = FeatureLevel;
        GPUInfo.VendorID = Desc.VendorId;
        pAdapter->QueryInterface(IID_PPV_ARGS(&GPUInfo.pAdapter));
        GPUs.push_back(GPUInfo);
    };
    
    // Enumerate Adapters
    while (pDXGIFactory->EnumAdapterByGpuPreference(iAdapter, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 Desc = {};
        pAdapter->GetDesc1(&Desc);

        const bool bSoftwareAdapter = Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
        if (
            (bEnumerateSoftwareAdapters && !bSoftwareAdapter) || // Need Software Adapters but get a hardware adapter
            (!bEnumerateSoftwareAdapters && bSoftwareAdapter) // Hardware Adapters but get a software adapter
        )
        {
            ++iAdapter;
            pAdapter->Release();
            continue;
        }

        HResult = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);
        if (SUCCEEDED(HResult))
        {
            FunctionAddAdapter(pAdapter, Desc, D3D_FEATURE_LEVEL_12_1);
        }
        else
        {
            const std::string AdapterDesc = WideToASCII(Desc.Description);
            Log::Warning("Device::Create() : Could not create D3D12Device with Feature_Level 12_1 for adapter %s, Trying with 12_0.", AdapterDesc.c_str());
            HResult = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr);
            if (SUCCEEDED(HResult))
            {
                FunctionAddAdapter(pAdapter, Desc, D3D_FEATURE_LEVEL_12_0);
            }
            else
            {
                Log::Error("Device::Create() : Could not create D3D12Device with Feature_Level 12_0 for adapter %s.", AdapterDesc.c_str());
            }
        }

        pAdapter->Release();
        ++iAdapter;
    }

    if (!pFactory)
    {
        pDXGIFactory->Release();
    }
    assert(bAdapterFound);
    return GPUs;
}

/**
 * Create a D3D12 device and enable the debug layer if requested.
 * @param CreateDesc A structure containing information about how to create the device.
 * @return True if the device was created successfully, false otherwise.
 */
bool Device::Create(const FDeviceCreateDesc& CreateDesc)
{
    HRESULT HResult = {};

    // Debug and Validation Layer check and create
    if (CreateDesc.bEnableDebugLayer)
    {
        ID3D12Debug1* pDebugController = nullptr;
        HResult = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController));
        if (HResult == S_OK)
        {
            pDebugController->EnableDebugLayer();
            if (CreateDesc.bEnableValidationLayer)
            {
                pDebugController->SetEnableGPUBasedValidation(true);
                pDebugController->SetEnableSynchronizedCommandQueueValidation(true);
            }
            pDebugController->Release();
            Log::Info("Device::Create() : Enable Debug %s", (CreateDesc.bEnableValidationLayer ? "and GPU Validation layers" : "layer"));
        }
        else
        {
            Log::Warning("Device::Create(): D3D12GetDebugInterface() returned != S_OK: %l", HResult);
        }
    }

    std::vector<FGPUInfo> Adapters = EnumerateDX12Adapters(CreateDesc.bEnableDebugLayer, CreateDesc.bEnableValidationLayer, CreateDesc.pFactory);
    assert(!Adapters.empty());

    FGPUInfo& Adapter = Adapters[0];

    this->mpAdapter = Adapter.pAdapter;
    // Create Device
    {
        HResult = D3D12CreateDevice(this->mpAdapter, Adapter.MaxSupportedFeatureLevel, IID_PPV_ARGS(&mpDevice));
        if (!SUCCEEDED(HResult))
        {
            Log::Error("Device::Create() : D3D12CreateDevice() failed");
            return false;
        }
    }
    // Create Device4
    {
        HResult = D3D12CreateDevice(this->mpAdapter, Adapter.MaxSupportedFeatureLevel, IID_PPV_ARGS(&mpDevice4));
        if (!SUCCEEDED(HResult))
        {
            Log::Error("Device::Create() : D3D12CreateDevice() failed");
        }
    }
    const bool bDeviceCreated = true;

    if (CreateDesc.bEnableDebugLayer)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        if (SUCCEEDED(mpDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue))))
        {
            D3D12_MESSAGE_ID IgnoredWarnings[] = {
                D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET
            };

            D3D12_INFO_QUEUE_FILTER Filter = {};
            Filter.DenyList.NumIDs = _countof(IgnoredWarnings);
            Filter.DenyList.pIDList = IgnoredWarnings;

            pInfoQueue->AddStorageFilterEntries(&Filter);
            pInfoQueue->Release();
        }
    }

    CheckDeviceFeautureSupport(this->mpDevice4, this->mCapabilities);
    return bDeviceCreated;
}

void Device::Destroy()
{
    mpAdapter->Release();
    mpDevice->Release();
    if (mpDevice4)
    {
        mpDevice4->Release();
    }
}

UINT Device::GetDeviceMemoryMax() const
{
    DXGI_ADAPTER_DESC Desc = {};
    if (mpAdapter)
    {
        mpAdapter->GetDesc(&Desc);
        return static_cast<UINT>(Desc.DedicatedVideoMemory / 1024 / 1024);
    }
    return 0;
}

UINT Device::GetDeviceMemoryAvailable() const
{
    IDXGIAdapter3* pAdapter3;
    if (SUCCEEDED(mpAdapter->QueryInterface(IID_PPV_ARGS(&pAdapter3))))
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO memInfo = {};
        pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
        return static_cast<UINT>(memInfo.Budget / 1024 / 1024);
    }
    return GetDeviceMemoryMax();
}