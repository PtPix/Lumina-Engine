/**
 * @file ResourceUploader.h
 * @brief GPU Resource Upload Manager.
 *
 * Handles staging memory (FUploadBuffer) and asynchronous GPU uploads
 * for buffers and textures via copy commands.
 */

#pragma once

#include <vector>
#include <queue>
#include <cstdint>

#include "Renderer/D3D12Core/Resource/Buffer.h"

class FDevice;
class FCommandQueue;
class FCommandContext;
class FTexture;

struct FUploadTask
{
    uint64_t FenceValue;
    std::vector<FUploadBuffer> TempUploadBuffers;
};

class FResourceUploader
{
public:
    void Initialize(FDevice* pDevice);

    // ------------------------------------------------------------------------
    // Upload Operations
    // ------------------------------------------------------------------------
    void BeginUpload();
    void QueueUpload(FBuffer* pDestBuffer, const void* pData, size_t DataSize);
    void UploadTexture(FTexture* pDestTexture, const void* pData, uint32_t Width, uint32_t Height, uint32_t BytesPerPixel);
    uint64_t EndUpLoadAndExecute();

    // ------------------------------------------------------------------------
    // Synchronization
    // ------------------------------------------------------------------------
    void CleanUpStaleUploads();
    void FlushAndSync();

private:
    FDevice* mpDevice = nullptr;
    FCommandQueue* mpCommandQueue = nullptr;
    FCommandContext* mpCurrentContext = nullptr;

    std::vector<FUploadBuffer> mCurrentTempUploadBuffers;
    std::queue<FUploadTask> mInFlightUploads;
};
