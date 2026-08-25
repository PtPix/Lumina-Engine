/**
 * @file D3D12ResourceUploader.h
 * @brief GPU Resource Upload Manager.
 *
 * Handles staging memory (FUploadBuffer) and asynchronous GPU uploads
 * for buffers and textures via copy commands.
 */

#pragma once

#include <vector>
#include <queue>
#include <cstdint>

#include "D3D12Buffer.h"

class FD3D12Device;
class FD3D12CommandQueue;
class FD3D12CommandContext;
class FD3D12Texture;

struct FD3D12UploadTask
{
    uint64_t FenceValue;
    std::vector<FD3D12UploadBuffer> TempUploadBuffers;
};

class FD3D12ResourceUploader
{
public:
    void Initialize(FD3D12Device* pDevice);

    // ------------------------------------------------------------------------
    // Upload Operations
    // ------------------------------------------------------------------------
    void QueueUpload(FD3D12Buffer* pDestBuffer, const void* pData, size_t DataSize);
    void UploadTexture(FD3D12Texture* pDestTexture, const void* pData, uint32_t Width, uint32_t Height, uint32_t BytesPerPixel);
    uint64_t SubmitPendingUploads();

    // ------------------------------------------------------------------------
    // Synchronization
    // ------------------------------------------------------------------------
    void CleanUpStaleUploads();
    void FlushAndSync();

private:
    FD3D12Device* mpDevice = nullptr;
    FD3D12CommandQueue* mpCommandQueue = nullptr;
    FD3D12CommandContext* mpCurrentContext = nullptr;

    std::vector<FD3D12UploadBuffer> mCurrentTempUploadBuffers;
    std::queue<FD3D12UploadTask> mInFlightUploads;
};
