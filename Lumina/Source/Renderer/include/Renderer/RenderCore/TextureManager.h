// #pragma once
// #include <d3d12.h>
// #include <string>
// #include <unordered_map>
//
// #include "D3D12MemAlloc.h"
// #include "Renderer/D3D12Core/ResourceHeaps.h"
// #include "../D3D12Core/Texture.h"
//
// class ResourceUploader;
// class FDevice;
// struct FTexture;
//
// class TextureManager
// {
// public:
//     TextureManager() = default;
//     ~TextureManager() { DestroyAll(); };
//
//     void Initialize(FDevice* pDevice);
//
//     // FTexture* GetOrCreateTexture(
//     //     const std::string& Name, const void* pData,
//     //     uint32_t Width, uint32_t Height,
//     //     DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM,
//     //     uint32_t BytesPerPixel = 4
//     //     );
//     FTexture* GetOrCreateTextureFromFile(const std::string& FilePath, ResourceUploader* pUploader);
//     FTexture* GetOrCreateTexture(FDevice* pDevice, ResourceUploader* pUploader, const std::string& Name, const void* pData, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, uint32_t BytesPerPixel);
//     void DestroyAll();
//
// private:
//     std::unordered_map<std::string, FTexture*> mTextureCache;
//
//     FDevice* mpDevice = nullptr;
//     D3D12MA::Allocator* mpAllocator = nullptr;
//     UploadHeap* mpUploadHeap = nullptr;
//     StaticResourceViewHeap* mpSrvHeap = nullptr;
// };
