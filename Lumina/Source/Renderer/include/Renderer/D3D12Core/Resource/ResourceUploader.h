#pragma once
#include "Buffer.h"
#include "Renderer/D3D12Core/Fence.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"

class ResourceUploader
{
public:
    void Initialize(D3D12MA::Allocator* pAllocator, FCommandContext* CopyContext, FCommandQueue* CopyQueue);
    void QueueUpload(Buffer* pDestBuffer, const void* pData, size_t DataSize);
    void FlushAndSync(ID3D12Device* pDevice);

private:
    D3D12MA::Allocator* mpAllocator = nullptr;
    FCommandContext* mCopyContext = nullptr;
    std::vector<UploadBuffer> mTempUploadBuffers;
    FCommandQueue* mCopyQueue = nullptr;

    ID3D12Fence* mpFence = nullptr;
    uint64_t mFenceValue = 0;
    HANDLE mHEvent = nullptr;
};
