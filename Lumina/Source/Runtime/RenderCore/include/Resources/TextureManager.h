#pragma once

#include "D3D12Texture.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <d3d12.h>

class FD3D12Device;
class FD3D12ResourceUploader;

struct FTextureData
{
    std::unique_ptr<FD3D12Texture> pTexture;
    uint32_t BindlessIndex = 0;
};

class TextureManager
{
public:
    static void Initialize(FD3D12Device* pDevice, FD3D12ResourceUploader* pUploader);
    static void Shutdown();

    static uint32_t LoadTexture(const std::string& FilePath, bool bIsSRGB = true);

    static uint32_t GetDefaultWhiteTexture() { return mDefaultWhiteIndex; }
    static uint32_t GetDefaultBlackTexture() { return mDefaultBlackIndex; }
    static uint32_t GetDefaultNormalTexture() { return mDefaultNormalIndex; }

private:
    static uint32_t CreateTextureFromData(const std::string& Name, const void* pData, uint32_t Width, uint32_t Height, DXGI_FORMAT Format);
    static void CreateDefaultTextures();

private:
    static FD3D12Device* mpDevice;
    static FD3D12ResourceUploader* mpUploader;

    static std::unordered_map<std::string, FTextureData> mTextureMap;

    static uint32_t mDefaultWhiteIndex;
    static uint32_t mDefaultBlackIndex;
    static uint32_t mDefaultNormalIndex;
};