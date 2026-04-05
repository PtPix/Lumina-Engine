#include "Renderer/RenderCore/TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Renderer/RenderCore/stb_image.h"

void TextureManager::Initialize(ID3D12Device* pDevice, D3D12MA::Allocator* pAllocator, UploadHeap* pUploadHeap,
                                StaticResourceViewHeap* pSrvHeap)
{
    mpDevice = pDevice;
    mpUploadHeap = pUploadHeap;
    mpSrvHeap = pSrvHeap;
    mpAllocator = pAllocator;
}

FTexture* TextureManager::GetOrCreateTexture(const std::string& Name, const void* pData, uint32_t Width,
    uint32_t Height, DXGI_FORMAT Format, uint32_t BytesPerPixel)
{
    // Find if exists
    auto FindResult = mTextureCache.find(Name);
    if (FindResult != mTextureCache.end())
    {
        Log::Info("Texture cache hit: %s", Name.c_str());
        return FindResult->second;
    }

    FTexture* NewTexture = new FTexture();
    NewTexture->Width = Width;
    NewTexture->Height = Height;
    NewTexture->Format = Format;

    D3D12_RESOURCE_DESC TextureDesc = {};
    TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    TextureDesc.Alignment = 0;
    TextureDesc.Width = Width;
    TextureDesc.Height = Height;
    TextureDesc.DepthOrArraySize = 1;
    TextureDesc.MipLevels = 1;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Format = Format;
    TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC AllocationDesc = {};
    AllocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    HRESULT HResult = mpAllocator->CreateResource(
        &AllocationDesc, &TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, &NewTexture->Allocation, IID_PPV_ARGS(&NewTexture->Resource)
        );
    if (FAILED(HResult))
    {
        Log::Error("Failed to create Texture Resource.");
        return nullptr;
    }

    std::wstring wName(Name.begin(), Name.end());
    NewTexture->Resource->SetName(wName.c_str());

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
    UINT NumRows;
    UINT64 RowSizeInBytes;
    UINT64 TotalBytes;
    mpDevice->GetCopyableFootprints(&TextureDesc, 0, 1, 0, &Footprint, &NumRows, &RowSizeInBytes, &TotalBytes);

    uint8_t* pUploadMemory = mpUploadHeap->SubAllocate(TotalBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
    if (!pUploadMemory)
    {
        Log::Error("UploadHeap out of memory for texture.");

        if (NewTexture->Resource)
        {
            NewTexture->Resource->Release();
        }
        if (NewTexture->Allocation)
        {
            NewTexture->Allocation->Release();
        }
        delete NewTexture;

        return nullptr;
    }

    const auto* pSourceData = static_cast<const uint8_t*>(pData);
    uint32_t SourceRowPitch = Width * BytesPerPixel;
    for (UINT y = 0; y < NumRows; y++)
    {
        memcpy(pUploadMemory + y * Footprint.Footprint.RowPitch,
            pSourceData + y * SourceRowPitch,
            SourceRowPitch);
    }
    ID3D12GraphicsCommandList* pCommandList = mpUploadHeap->GetCommandList();
    D3D12_TEXTURE_COPY_LOCATION DestLocation = {};
    DestLocation.pResource = NewTexture->Resource;
    DestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    DestLocation.SubresourceIndex = 0;
    D3D12_TEXTURE_COPY_LOCATION SourceLocation = {};
    SourceLocation.pResource = mpUploadHeap->GetResource();
    SourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    SourceLocation.PlacedFootprint = Footprint;
    SourceLocation.PlacedFootprint.Offset += (pUploadMemory - mpUploadHeap->DataBegin());

    pCommandList->CopyTextureRegion(&DestLocation, 0, 0, 0, &SourceLocation, nullptr);

    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Transition.pResource = NewTexture->Resource;
    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    pCommandList->ResourceBarrier(1, &Barrier);

    if (!mpSrvHeap->AllocateDescriptor(1, &NewTexture->SourceView))
    {
        Log::Error("Failed to allocate SRV descriptor for texture.");

        if (NewTexture->Resource)
        {
            NewTexture->Resource->Release();
        }
        if (NewTexture->Allocation)
        {
            NewTexture->Allocation->Release();
        }
        delete NewTexture;

        return nullptr;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = {};
    ShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    ShaderResourceViewDesc.Format = Format;
    ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceViewDesc.Texture2D.MipLevels = 1;
    mpDevice->CreateShaderResourceView(NewTexture->Resource, &ShaderResourceViewDesc, NewTexture->SourceView.GetCpuDescriptorHandle());

    mTextureCache[Name] = NewTexture;
    Log::Info("Created and cached new texture: %s", Name.c_str());

    return NewTexture;
}

FTexture* TextureManager::GetOrCreateTextureFromFile(const std::string& FilePath)
{
    auto it = mTextureCache.find(FilePath);
    if (it != mTextureCache.end())
    {
        Log::Info("Texture cache hit: %s", FilePath.c_str());
        return it->second;
    }

    int Width, Height, Channels;
    stbi_uc* pPixels = stbi_load(FilePath.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);

    if (!pPixels)
    {
        Log::Error("Failed to load texture file: %s", FilePath.c_str());
        return nullptr;
    }

    FTexture* NewTexture = GetOrCreateTexture(
        FilePath, pPixels,
        static_cast<uint32_t>(Width),
        static_cast<uint32_t>(Height),
        DXGI_FORMAT_R8G8B8A8_UNORM,
        4
        );

    stbi_image_free(pPixels);
    return NewTexture;
}

void TextureManager::DestroyAll()
{
    for (auto& Cache : mTextureCache)
    {
        FTexture* pTexture = Cache.second;
        if (pTexture)
        {
            if (pTexture->Resource)
            {
                pTexture->Resource->Release();
            }
            if (pTexture->Allocation)
            {
                pTexture->Allocation->Release();
            }
            if (mpSrvHeap)
            {
                mpSrvHeap->FreeDescriptor(&pTexture->SourceView);
            }
            delete pTexture;
        }
    }
    mTextureCache.clear();
}
