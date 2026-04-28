#pragma once
#include "Buffer.h"
#include "FTexture.h"
#include "Renderer/D3D12Core/Fence.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"

class ResourceUploader
{
public:
    void Initialize(D3D12MA::Allocator* pAllocator, FCommandContext* CopyContext, FCommandQueue* CopyQueue);
    void QueueUpload(FBuffer* pDestBuffer, const void* pData, size_t DataSize);
    void UploadTexture(FDevice* pDevice, FTexture* pDestTexture, const void* pData, uint32_t Width, uint32_t Height, uint32_t BytesPerPixel);
    void FlushAndSync();

private:
    D3D12MA::Allocator* mpAllocator = nullptr;
    FCommandContext* mCopyContext = nullptr;
    std::vector<FUploadBuffer> mTempUploadBuffers;
    FCommandQueue* mCopyQueue = nullptr;

    ID3D12Fence* mpFence = nullptr;
    uint64_t mFenceValue = 0;
    HANDLE mHEvent = nullptr;
};
