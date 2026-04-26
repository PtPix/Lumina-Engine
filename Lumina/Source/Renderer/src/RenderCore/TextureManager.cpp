// #include "Renderer/RenderCore/TextureManager.h"
//
// #define STB_IMAGE_IMPLEMENTATION
// #include "../../include/Renderer/D3D12Core/stb_image.h"
// #include "Renderer/D3D12Core/Core/FDevice.h"
// #include "Renderer/D3D12Core/Resource/ResourceUploader.h"
// #include "Renderer/D3D12Core/Resource/Texture.h"
//
// void TextureManager::Initialize(ID3D12Device* pDevice, D3D12MA::Allocator* pAllocator, UploadHeap* pUploadHeap,
//                                 StaticResourceViewHeap* pSrvHeap)
// {
//     mpDevice = pDevice;
//     mpUploadHeap = pUploadHeap;
//     mpSrvHeap = pSrvHeap;
//     mpAllocator = pAllocator;
// }
//
// FTexture* TextureManager::GetOrCreateTexture(const std::string& Name, const void* pData, uint32_t Width,
//     uint32_t Height, DXGI_FORMAT Format, uint32_t BytesPerPixel)
// {
//     // Find if exists
//     auto FindResult = mTextureCache.find(Name);
//     if (FindResult != mTextureCache.end())
//     {
//         Log::Info("Texture cache hit: %s", Name.c_str());
//         return FindResult->second;
//     }
//
//     FTexture* NewTexture = new FTexture();
//     NewTexture->Width = Width;
//     NewTexture->Height = Height;
//     NewTexture->Format = Format;
//
//     D3D12_RESOURCE_DESC TextureDesc = {};
//     TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//     TextureDesc.Alignment = 0;
//     TextureDesc.Width = Width;
//     TextureDesc.Height = Height;
//     TextureDesc.DepthOrArraySize = 1;
//     TextureDesc.MipLevels = 1;
//     TextureDesc.SampleDesc.Count = 1;
//     TextureDesc.SampleDesc.Quality = 0;
//     TextureDesc.Format = Format;
//     TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//     TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//     D3D12MA::ALLOCATION_DESC AllocationDesc = {};
//     AllocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
//     HRESULT HResult = mpAllocator->CreateResource(
//         &AllocationDesc, &TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
//         nullptr, &NewTexture->Allocation, IID_PPV_ARGS(&NewTexture->Resource)
//         );
//     if (FAILED(HResult))
//     {
//         LUMINA_LOG_ERROR(RHI, "Failed to create Texture Resource.");
//         return nullptr;
//     }
//
//     std::wstring wName(Name.begin(), Name.end());
//     NewTexture->Resource->SetName(wName.c_str());
//
//     D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
//     UINT NumRows;
//     UINT64 RowSizeInBytes;
//     UINT64 TotalBytes;
//     mpDevice->GetCopyableFootprints(&TextureDesc, 0, 1, 0, &Footprint, &NumRows, &RowSizeInBytes, &TotalBytes);
//
//     uint8_t* pUploadMemory = mpUploadHeap->SubAllocate(TotalBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
//     if (!pUploadMemory)
//     {
//         LUMINA_LOG_ERROR(RHI, "UploadHeap out of memory for texture.");
//
//         if (NewTexture->Resource)
//         {
//             NewTexture->Resource->Release();
//         }
//         if (NewTexture->Allocation)
//         {
//             NewTexture->Allocation->Release();
//         }
//         delete NewTexture;
//
//         return nullptr;
//     }
//
//     const auto* pSourceData = static_cast<const uint8_t*>(pData);
//     uint32_t SourceRowPitch = Width * BytesPerPixel;
//     for (UINT y = 0; y < NumRows; y++)
//     {
//         memcpy(pUploadMemory + y * Footprint.Footprint.RowPitch,
//             pSourceData + y * SourceRowPitch,
//             SourceRowPitch);
//     }
//     ID3D12GraphicsCommandList* pCommandList = mpUploadHeap->GetCommandList();
//     D3D12_TEXTURE_COPY_LOCATION DestLocation = {};
//     DestLocation.pResource = NewTexture->Resource;
//     DestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
//     DestLocation.SubresourceIndex = 0;
//     D3D12_TEXTURE_COPY_LOCATION SourceLocation = {};
//     SourceLocation.pResource = mpUploadHeap->GetResource();
//     SourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//     SourceLocation.PlacedFootprint = Footprint;
//     SourceLocation.PlacedFootprint.Offset += (pUploadMemory - mpUploadHeap->DataBegin());
//
//     pCommandList->CopyTextureRegion(&DestLocation, 0, 0, 0, &SourceLocation, nullptr);
//
//     D3D12_RESOURCE_BARRIER Barrier = {};
//     Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//     Barrier.Transition.pResource = NewTexture->Resource;
//     Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//     Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//     Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//     pCommandList->ResourceBarrier(1, &Barrier);
//
//     if (!mpSrvHeap->AllocateDescriptor(1, &NewTexture->SourceView))
//     {
//         LUMINA_LOG_ERROR(RHI, "Failed to allocate SRV descriptor for texture.");
//
//         if (NewTexture->Resource)
//         {
//             NewTexture->Resource->Release();
//         }
//         if (NewTexture->Allocation)
//         {
//             NewTexture->Allocation->Release();
//         }
//         delete NewTexture;
//
//         return nullptr;
//     }
//
//     D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = {};
//     ShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//     ShaderResourceViewDesc.Format = Format;
//     ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//     ShaderResourceViewDesc.Texture2D.MipLevels = 1;
//     mpDevice->CreateShaderResourceView(NewTexture->Resource, &ShaderResourceViewDesc, NewTexture->SourceView.GetCpuDescriptorHandle());
//
//     mTextureCache[Name] = NewTexture;
//     Log::Info("Created and cached new texture: %s", Name.c_str());
//
//     return NewTexture;
// }
//
// FTexture* TextureManager::GetOrCreateTextureFromFile(const std::string& FilePath)
// {
//     auto it = mTextureCache.find(FilePath);
//     if (it != mTextureCache.end())
//     {
//         Log::Info("Texture cache hit: %s", FilePath.c_str());
//         return it->second;
//     }
//
//     int Width, Height, Channels;
//     stbi_uc* pPixels = stbi_load(FilePath.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);
//
//     if (!pPixels)
//     {
//         Log::Error("Failed to load texture file: %s", FilePath.c_str());
//         return nullptr;
//     }
//
//     FTexture* NewTexture = GetOrCreateTexture(
//         FilePath, pPixels,
//         static_cast<uint32_t>(Width),
//         static_cast<uint32_t>(Height),
//         DXGI_FORMAT_R8G8B8A8_UNORM,
//         4
//         );
//
//     stbi_image_free(pPixels);
//     return NewTexture;
// }
//
// Texture* TextureManager::GetOrCreateTexture(FDevice* pDevice, ResourceUploader* pUploader, const std::string& Name,
//     const void* pData, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, uint32_t BytesPerPixel)
// {
//     auto FindResult = mTextureCache.find(Name);
//     if (FindResult != mTextureCache.end())
//     {
//         Log::Info("Texture cache hit: %s", Name.c_str());
//         return FindResult->second; // 这里的类型已经变成了 Texture*
//     }
//
//     // 2. 配置并申请物理显存资源
//     D3D12_RESOURCE_DESC TextureDesc = {};
//     TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//     TextureDesc.Width = Width;
//     TextureDesc.Height = Height;
//     TextureDesc.DepthOrArraySize = 1;
//     TextureDesc.MipLevels = 1;
//     TextureDesc.SampleDesc.Count = 1;
//     TextureDesc.Format = Format;
//
//     D3D12MA::ALLOCATION_DESC AllocDesc = {};
//     AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
//
//     Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
//     D3D12MA::Allocation* pAllocation = nullptr;
//
//     HRESULT HResult = pDevice->GetAllocator()->CreateResource(
//         &AllocDesc, &TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
//         nullptr, &pAllocation, IID_PPV_ARGS(&pResource)
//     );
//
//     if (FAILED(HResult)) return nullptr;
//
//     std::wstring wName(Name.begin(), Name.end());
//     pResource->SetName(wName.c_str());
//
//     // 3. 创建面向对象的新 Texture 并初始化！(SRV 在这里自动生成了)
//     Texture* NewTexture = new Texture();
//     NewTexture->Create2D(pDevice, Width, Height, Format, pAllocation, pResource.Get());
//
//     // 4. 交给 Uploader 去处理所有的 Copy 和 屏障转换逻辑
//     if (pData && pUploader)
//     {
//         pUploader->UploadTexture(pDevice, NewTexture, pData, Width, Height, BytesPerPixel);
//     }
//
//     // 5. 存入缓存
//     mTextureCache[Name] = NewTexture;
//     Log::Info("Created and cached new texture: %s", Name.c_str());
//
//     return NewTexture;
// }
//
// void TextureManager::DestroyAll()
// {
//     for (auto& Cache : mTextureCache)
//     {
//         Texture* pTexture = Cache.second;
//         if (pTexture)
//         {
//             if (pTexture->GetResource())
//             {
//                 pTexture->GetResource()->Release();
//             }
//             // if (pTextureAllocation)
//             // {
//             //     pTexture->Allocation->Release();
//             // }
//             // if (mpSrvHeap)
//             // {
//             //     mpSrvHeap->FreeDescriptor(&pTexture->SourceView);
//             // }
//             delete pTexture;
//         }
//     }
//     mTextureCache.clear();
// }
#include "Renderer/RenderCore/TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/Renderer/D3D12Core/stb_image.h"
#include "Renderer/D3D12Core/Resource/Texture.h"
#include "Renderer/D3D12Core/Resource/ResourceUploader.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Logger/Logger.h"

// ============================================================================
// 1. 极简初始化
// ============================================================================
void TextureManager::Initialize(FDevice* pDevice)
{
    mpDevice = pDevice;
    // 不再需要保存 Allocator、UploadHeap 和 StaticSrvHeap
    // 因为 FDevice 内部自带了 Allocator 和 DescriptorManager
}

// ============================================================================
// 2. 从文件加载 (自动调用内存加载)
// ============================================================================
Texture* TextureManager::GetOrCreateTextureFromFile(const std::string& FilePath, ResourceUploader* pUploader)
{
    auto it = mTextureCache.find(FilePath);
    if (it != mTextureCache.end())
    {
        Log::Info("Texture cache hit: %s", FilePath.c_str());
        return it->second;
    }

    int Width, Height, Channels;
    // 使用 stb_image 读取图片到内存
    stbi_uc* pPixels = stbi_load(FilePath.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);

    if (!pPixels)
    {
        Log::Error("Failed to load texture file: %s", FilePath.c_str());
        return nullptr;
    }

    // 调用核心的内存创建函数
    Texture* NewTexture = GetOrCreateTexture(
        mpDevice,
        pUploader,
        FilePath,
        pPixels,
        static_cast<uint32_t>(Width),
        static_cast<uint32_t>(Height),
        DXGI_FORMAT_R8G8B8A8_UNORM, // stb_image 以 4 通道 rgba 读取，因此是 R8G8B8A8
        4                           // BytesPerPixel = 4
    );

    // 释放 stb_image 的临时内存
    stbi_image_free(pPixels);
    return NewTexture;
}

// ============================================================================
// 3. 核心内存创建逻辑 (面向对象 + 动态上传)
// ============================================================================
Texture* TextureManager::GetOrCreateTexture(FDevice* pDevice, ResourceUploader* pUploader, const std::string& Name,
    const void* pData, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, uint32_t BytesPerPixel)
{
    auto FindResult = mTextureCache.find(Name);
    if (FindResult != mTextureCache.end())
    {
        Log::Info("Texture cache hit: %s", Name.c_str());
        return FindResult->second;
    }

    // 1. 配置纹理描述符
    D3D12_RESOURCE_DESC TextureDesc = {};
    TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    TextureDesc.Width = Width;
    TextureDesc.Height = Height;
    TextureDesc.DepthOrArraySize = 1;
    TextureDesc.MipLevels = 1;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Format = Format;

    D3D12MA::ALLOCATION_DESC AllocDesc = {};
    AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT; // 纹理必须放在 DEFAULT 堆 (GPU 显存)

    Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
    D3D12MA::Allocation* pAllocation = nullptr;

    // 向系统请求物理显存
    HRESULT HResult = pDevice->GetAllocator()->CreateResource(
        &AllocDesc, &TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, &pAllocation, IID_PPV_ARGS(&pResource)
    );

    if (FAILED(HResult))
    {
        Log::Error("Failed to allocate memory for Texture: %s", Name.c_str());
        return nullptr;
    }

    std::wstring wName(Name.begin(), Name.end());
    pResource->SetName(wName.c_str());

    // 2. 创建面向对象的新 Texture 并初始化 (这一步会自动申请永久的 SRV CPU 坑位)
    Texture* NewTexture = new Texture();
    NewTexture->Create2D(pDevice, Width, Height, Format, pAllocation, pResource.Get());

    // 3. 交给 Uploader 去处理拷贝、屏障状态转换、和生命周期续命
    if (pData && pUploader)
    {
        pUploader->UploadTexture(pDevice, NewTexture, pData, Width, Height, BytesPerPixel);
    }

    // 4. 存入缓存
    mTextureCache[Name] = NewTexture;
    Log::Info("Created and cached new texture: %s", Name.c_str());

    return NewTexture;
}

// ============================================================================
// 4. 极致清爽的析构 (RAII 的魔力)
// ============================================================================
void TextureManager::DestroyAll()
{
    for (auto& CachePair : mTextureCache)
    {
        Texture* pTexture = CachePair.second;
        if (pTexture)
        {
            pTexture->Destroy();
            // 只需要一个 delete！
            // Texture 的析构函数会自动调用 ComPtr 的 Release() 释放底层 Resource。
            // 它的成员 mSRV (FDescriptorAllocation) 会自动触发析构，将坑位退还给 FDescriptorAllocatorPage。
            // 没有任何显式的 Release 和 FreeDescriptor 代码！
            delete pTexture;
        }
    }
    mTextureCache.clear();
}