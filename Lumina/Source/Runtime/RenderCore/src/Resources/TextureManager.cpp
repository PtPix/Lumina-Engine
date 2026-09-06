#include "Resources/TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../ThirdParty/stb_image.h"
#include "D3D12Backend.h"
#include "D3D12Device.h"
#include "D3D12ResourceUploader.h"
#include "D3D12BindlessDescriptorHeap.h"
#include "Logger/Logger.h"

FD3D12Device* TextureManager::mpDevice = nullptr;
std::unordered_map<std::string, FTextureData> TextureManager::mTextureMap;
uint32_t TextureManager::mDefaultWhiteIndex = 0;
uint32_t TextureManager::mDefaultBlackIndex = 0;
uint32_t TextureManager::mDefaultNormalIndex = 0;
FD3D12ResourceUploader* TextureManager::mpUploader = nullptr;

void TextureManager::Initialize(FD3D12Device* pDevice, FD3D12ResourceUploader* pUploader)
{
    mpDevice = pDevice;
    mpUploader = pUploader;

    CreateDefaultTextures();
}

void TextureManager::Shutdown()
{
    mTextureMap.clear();
}

uint32_t TextureManager::LoadTexture(const std::string& FilePath, bool bIsSRGB)
{
    if (mTextureMap.find(FilePath) != mTextureMap.end())
    {
        return mTextureMap[FilePath].BindlessIndex;
    }

    int Width, Height, Channels;

    stbi_uc* pPixels = stbi_load(FilePath.c_str(), &Width, &Height, &Channels, 4);

    if (!pPixels)
    {
        LUMINA_LOG_ERROR(Texture, "Failed to load Textures: %s", FilePath.c_str());
        return GetDefaultWhiteTexture();
    }

    DXGI_FORMAT Format = bIsSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    uint32_t BindlessIndex = CreateTextureFromData(FilePath, pPixels, Width, Height, Format);

    stbi_image_free(pPixels);

    return BindlessIndex;
}

uint32_t TextureManager::CreateTextureFromData(const std::string& Name, const void* pData, uint32_t Width,
    uint32_t Height, DXGI_FORMAT Format)
{
    auto pTexture = std::make_unique<FD3D12Texture>();

    FD3D12TextureDesc Desc = {};
    Desc.Dimension        = ED3D12TextureDimension::Texture2D;
    Desc.Width            = Width;
    Desc.Height           = Height;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels        = 1;
    Desc.Format           = Format;
    Desc.Flags            = ED3D12TextureFlags::None;
    Desc.InitialState     = D3D12_RESOURCE_STATE_COMMON;
    Desc.DebugName        = std::wstring(Name.begin(), Name.end());

    if (!pTexture->Create(mpDevice, mpDevice->GetAllocator(), Desc))
    {
        LUMINA_LOG_ERROR(Texture, "Failed to create Texture: %s", Name.c_str());
        return 0;
    }

    mpUploader->UploadTexture(pTexture.get(), pData, Width, Height, 4);

    FD3D12BindlessDescriptorHeap* pBindlessHeap = mpDevice->GetBindlessDescriptorHeap();
    uint32_t BindlessIndex = pBindlessHeap->AllocateSlot();
    pBindlessHeap->CreateSRVFromCPUHandle(mpDevice, pTexture->GetSRV(), BindlessIndex);

    FTextureData TextureData;
    TextureData.pTexture = std::move(pTexture);
    TextureData.BindlessIndex = BindlessIndex;
    mTextureMap[Name] = std::move(TextureData);

    return BindlessIndex;
}

void TextureManager::CreateDefaultTextures()
{
    uint8_t WhiteData[4] = { 255, 255, 255, 255 };
    mDefaultWhiteIndex = CreateTextureFromData("Default_White", WhiteData, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

    uint8_t BlackData[4] = { 0, 0, 0, 255 };
    mDefaultBlackIndex = CreateTextureFromData("Default_Black", BlackData, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

    uint8_t NormalData[4] = { 128, 128, 255, 255 };
    mDefaultNormalIndex = CreateTextureFromData("Default_Normal", NormalData, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
}