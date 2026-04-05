#pragma once
#include <d3d12.h>
#include <string>
#include <unordered_map>

#include "D3D12MemAlloc.h"
#include "ResourceHeaps.h"
#include "Texture.h"

class TextureManager
{
public:
    TextureManager() = default;
    ~TextureManager() { DestroyAll(); };

    void Initialize(ID3D12Device* pDevice, D3D12MA::Allocator* pAllocator, UploadHeap* pUploadHeap, StaticResourceViewHeap* pSrvHeap);

    FTexture* GetOrCreateTexture(
        const std::string& Name, const void* pData,
        uint32_t Width, uint32_t Height,
        DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        uint32_t BytesPerPixel = 4
        );
    FTexture* GetOrCreateTextureFromFile(const std::string& FilePath);

    void DestroyAll();

private:
    std::unordered_map<std::string, FTexture*> mTextureCache;

    ID3D12Device* mpDevice = nullptr;
    D3D12MA::Allocator* mpAllocator = nullptr;
    UploadHeap* mpUploadHeap = nullptr;
    StaticResourceViewHeap* mpSrvHeap = nullptr;
};
