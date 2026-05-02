#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Common.h"

struct FGpuInfo
{
    std::string DeviceName;
    unsigned DeviceID;
    unsigned VendorID;
    size_t DedicatedGPUMemory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;
    D3D_FEATURE_LEVEL MaxSupportedFeatureLevel;

    [[nodiscard]] bool IsAMD() const { return (VendorID == 0x1002); }
    [[nodiscard]] bool IsNVIDIA() const { return (VendorID == 0x10DE); }
    [[nodiscard]] bool IsIntel() const { return (VendorID == 0x163C) || (VendorID == 0x8086) || (VendorID == 0x8087); }
};

static void CheckDeviceFeatureSupport(ID3D12Device* pDevice, FDeviceCapabilities& DeviceCapabilities)
{
    HRESULT HResult;

    // Check if hardware ray tracing is supported
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 FeatureOptions5 = {};
        HResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &FeatureOptions5, sizeof(FeatureOptions5));
        if (!SUCCEEDED(HResult))
        {
            LUMINA_LOG_WARNING(RHI, "FDevice::CheckFeatureSupport() : Hardware ray tracing failed.");
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
            LUMINA_LOG_WARNING(RHI, "FDevice::CheckFeatureSupport() : Wave optimization failed.");
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
            LUMINA_LOG_WARNING(RHI, "FDevice::CheckFeatureSupport() : Half Precision Float failed.");
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
            LUMINA_LOG_WARNING(RHI, "FDevice::CheckFeatureSupport() : Mesh Shaders and Sampler Feedback failed.");
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
            // Log::Warning("FDevice::CheckFeatureSupport() : Enhanced Barrier failed.");
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
            LUMINA_LOG_WARNING(RHI, "FDevice::CheckFeatureSupport() : TypedUAVLoads failed.");
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
                        // Log::Warning("FDevice::CheckFeatureSupport() : TypedUAVLoads for DXGI_FORMAT:%d failed.", Format);
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
            // Log::Warning("FDevice::CheckFeatureSupport() : MSAA Quality failed.");
        }
        else
        {
            DeviceCapabilities.SupportedMaxMultiSampleQualityLevel = FeatureMSAAQualityLevels.NumQualityLevels;
        }
    }
}

static std::vector<FGpuInfo> EnumerateDX12Adapters(bool bEnableDebugLayer, bool bEnumerateSoftwareAdapters, IDXGIFactory6* pFactory)
{
    std::vector<FGpuInfo> GPUs;
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
        CreateDXGIFactory2(DXGIFlags, IID_PPV_ARGS(&pDXGIFactory));
    }

    // Lambda function for adding adapters to GPUs
    auto FunctionAddAdapter = [&bAdapterFound, &GPUs](IDXGIAdapter1* pAdapter, const DXGI_ADAPTER_DESC1& Desc, D3D_FEATURE_LEVEL FeatureLevel)
    {
        bAdapterFound = true;

        FGpuInfo GPUInfo = {};
        GPUInfo.DedicatedGPUMemory = Desc.DedicatedVideoMemory;
        GPUInfo.DeviceID = Desc.DeviceId;
        GPUInfo.DeviceName = StringUtils::WideToUTF8(Desc.Description);
        GPUInfo.MaxSupportedFeatureLevel = FeatureLevel;
        GPUInfo.VendorID = Desc.VendorId;
        GPUInfo.pAdapter = pAdapter;

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
            const std::string AdapterDesc = StringUtils::WideToUTF8(Desc.Description);
            Log::Warning("FDevice::Create() : Could not create D3D12Device with Feature_Level 12_1 for adapter %s, Trying with 12_0.", AdapterDesc.c_str());
            HResult = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr);
            if (SUCCEEDED(HResult))
            {
                FunctionAddAdapter(pAdapter, Desc, D3D_FEATURE_LEVEL_12_0);
            }
            else
            {
                Log::Error("FDevice::Create() : Could not create D3D12Device with Feature_Level 12_0 for adapter %s.", AdapterDesc.c_str());
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

bool FDevice::Create(const FDeviceCreateDesc& CreateDesc)
{
    HRESULT HResult = {};

    // Debug and Validation Layer check and create
    if (CreateDesc.bEnableDebugLayer)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug1> pDebugController;
        HResult = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController));
        if (HResult == S_OK)
        {
            pDebugController->EnableDebugLayer();
            if (CreateDesc.bEnableValidationLayer)
            {
                pDebugController->SetEnableGPUBasedValidation(true);
                pDebugController->SetEnableSynchronizedCommandQueueValidation(true);
            }
            LUMINA_LOG_INFO(RHI, "FDevice::Create() : Enable Debug %s", (CreateDesc.bEnableValidationLayer ? "and GPU Validation layers" : "layer"));
        }
        else
        {
            LUMINA_LOG_WARNING(RHI, "FDevice::Create(): D3D12GetDebugInterface() returned != S_OK");
        }
    }

    std::vector<FGpuInfo> Adapters = EnumerateDX12Adapters(CreateDesc.bEnableDebugLayer, CreateDesc.bEnableValidationLayer, CreateDesc.pFactory);
    assert(!Adapters.empty());

    FGpuInfo& Adapter = Adapters[0];
    mpAdapter = Adapter.pAdapter;

    // Create FDevice
    {
        HResult = D3D12CreateDevice(mpAdapter.Get(), Adapter.MaxSupportedFeatureLevel, IID_PPV_ARGS(&mpDevice));
        if (!SUCCEEDED(HResult))
        {
            LUMINA_LOG_ERROR(RHI, "FDevice::Create() : D3D12CreateDevice() failed");
            return false;
        }
    }
    constexpr bool bDeviceCreated = true;

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

    CheckDeviceFeatureSupport(this->mpDevice.Get(), this->mCapabilities);

    // Create D3D12MA
    {
        LUMINA_TIME_LOG_SCOPE("Create D3D12MA");
        D3D12MA::ALLOCATOR_DESC AllocatorDesc = {};
        AllocatorDesc.pDevice = mpDevice.Get();
        AllocatorDesc.pAdapter = mpAdapter.Get();
        HResult = D3D12MA::CreateAllocator(&AllocatorDesc, &mpAllocator);
        if (FAILED(HResult))
        {
            LUMINA_LOG_ERROR(RHI, "FDevice::Create() : Failed to create D3D12MA Allocator");
            return false;
        }
    }

    // Create Command Queues
    {
        LUMINA_TIME_LOG_SCOPE("Create Command Queues");

        mpGraphicsQueue->Create(this, GRAPHICS);
        SetName(mpGraphicsQueue->GetCommandQueue(), "Rendering Graphics Command Queue");
        mpComputeQueue->Create(this, COMPUTE);
        SetName(mpComputeQueue->GetCommandQueue(), "Rendering Compute Command Queue");
        mpCopyQueue->Create(this, COPY);
        SetName(mpCopyQueue->GetCommandQueue(), "Rendering Copy Command Queue");
    }

    // Create Cpu Descriptor Allocators
    {
        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            auto HeapType = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i);
            UINT PageSize = (HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? 256 : 64;

            mpDescriptorAllocators[i] = std::make_unique<FDescriptorAllocator>(HeapType, PageSize);
            mpDescriptorAllocators[i]->Initialize(this);
        }
    }

    // Create Bindless Descriptor Heap
    {
        LUMINA_TIME_LOG_SCOPE("Create Bindless Descriptor Heap");
        mpBindlessHeap->Initialize(this);
    }

    return bDeviceCreated;
}

void FDevice::Destroy()
{
    mpGraphicsQueue.reset();
    mpComputeQueue.reset();
    mpCopyQueue.reset();

    mpBindlessHeap.reset();

    for (auto & mpDescriptorAllocator : mpDescriptorAllocators)
    {
        mpDescriptorAllocator.reset();
    }

    mpAllocator.Reset();
    mpDevice.Reset();
}

UINT FDevice::GetDeviceMemoryMax() const
{
    DXGI_ADAPTER_DESC Desc = {};
    if (mpAdapter)
    {
        mpAdapter->GetDesc(&Desc);
        return static_cast<UINT>(Desc.DedicatedVideoMemory / 1024 / 1024);
    }
    return 0;
}

UINT FDevice::GetDeviceMemoryAvailable() const
{
    Microsoft::WRL::ComPtr<IDXGIAdapter3> pAdapter3;
    if (SUCCEEDED(mpAdapter->QueryInterface(IID_PPV_ARGS(&pAdapter3))))
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO memInfo = {};
        pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
        return static_cast<UINT>(memInfo.Budget / 1024 / 1024);
    }
    return GetDeviceMemoryMax();
}