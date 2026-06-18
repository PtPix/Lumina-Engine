#include "Renderer/Managers/TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

FDevice* TextureManager::mpDevice = nullptr;
std::unordered_map<std::string, FTextureData> TextureManager::mTextureMap;
uint32_t TextureManager::mDefaultWhiteIndex = 0;
uint32_t TextureManager::mDefaultBlackIndex = 0;
uint32_t TextureManager::mDefaultNormalIndex = 0;
FResourceUploader* TextureManager::mpUploader = nullptr;

void TextureManager::Initialize(FDevice* pDevice, FResourceUploader* pUploader)
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
    auto pTexture = std::make_unique<FTexture>();
    std::wstring wName(Name.begin(), Name.end());

    pTexture->Create(mpDevice, mpDevice->GetAllocator(), Width, Height,
        Format, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON,
        nullptr, wName);

    mpUploader->UploadTexture(pTexture.get(), pData, Width, Height, 4);

    FBindlessDescriptorHeap* pBindlessHeap = mpDevice->GetBindlessDescriptorHeap();
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