#pragma once

#include <vector>
#include <queue>

#include "Renderer/D3D12Core/Resource/FBuffer.h"
#include "Renderer/D3D12Core/Resource/FTexture.h"

class FDevice;
class FCommandQueue;
class FCommandContext;

struct FUploadTask
{
    uint64_t FenceValue;
    std::vector<FUploadBuffer> TempUploadBuffers;
};

class FResourceUploader
{
public:
    void Initialize(FDevice* pDevice);

    void BeginUpload();
    void QueueUpload(FBuffer* pDestBuffer, const void* pData, size_t DataSize);
    void UploadTexture(FTexture* pDestTexture, const void* pData, uint32_t Width, uint32_t Height, uint32_t BytesPerPixel);
    uint64_t EndUpLoadAndExecute();

    void CleanUpStaleUploads();

    void FlushAndSync();

private:
    FDevice* mpDevice = nullptr;
    FCommandQueue* mpCommandQueue = nullptr;

    FCommandContext* mpCurrentContext = nullptr;

    std::vector<FUploadBuffer> mCurrentTempUploadBuffers;

    std::queue<FUploadTask> mInFlightUploads;
};
