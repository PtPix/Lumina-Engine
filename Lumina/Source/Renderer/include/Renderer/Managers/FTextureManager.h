#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <d3d12.h>

#include "Renderer/D3D12Core/Resource/FTexture.h"

class FDevice;
class FResourceUploader;

struct FTextureData
{
    std::unique_ptr<FTexture> pTexture;
    uint32_t BindlessIndex = 0;
};

class TextureManager
{
public:
    static void Initialize(FDevice* pDevice, FResourceUploader* pUploader);
    static void Shutdown();

    static uint32_t LoadTexture(const std::string& FilePath, FResourceUploader* pUploader, bool bIsSRGB = true);

    static uint32_t GetDefaultWhiteTexture() { return mDefaultWhiteIndex; }
    static uint32_t GetDefaultBlackTexture() { return mDefaultBlackIndex; }
    static uint32_t GetDefaultNormalTexture() { return mDefaultNormalIndex; }

private:
    static uint32_t CreateTextureFromData(const std::string& Name, const void* pData, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, FResourceUploader* pUploader);
    static void CreateDefaultTextures(FResourceUploader* pUploader);

private:
    static FDevice* mpDevice;

    static std::unordered_map<std::string, FTextureData> mTextureMap;

    static uint32_t mDefaultWhiteIndex;
    static uint32_t mDefaultBlackIndex;
    static uint32_t mDefaultNormalIndex;
};