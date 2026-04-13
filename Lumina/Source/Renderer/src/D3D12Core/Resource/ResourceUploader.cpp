#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

void ResourceUploader::Initialize(D3D12MA::Allocator* pAllocator, FCommandContext* pCopyContext, FCommandQueue* pCopyQueue)
{
    mpAllocator = pAllocator;
    mCopyContext = pCopyContext;
    mCopyQueue = pCopyQueue;
    mCopyContext->Begin();
}

void ResourceUploader::QueueUpload(Buffer* pDestBuffer, const void* pData, size_t DataSize)
{
    UploadBuffer TempBuffer;
    TempBuffer.Create(mpAllocator, DataSize, L"TempUpload");

    void* pMapped = TempBuffer.Map();
    memcpy(pMapped, pData, DataSize);
    TempBuffer.Unmap();

    mCopyContext->TransitionResource(pDestBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    mCopyContext->FlushResourceBarriers();
    mCopyContext->CopyBufferRegion(pDestBuffer->GetResource(), 0, TempBuffer.GetResource(), 0, DataSize);
    mCopyContext->TransitionResource(pDestBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

    mTempUploadBuffers.push_back(std::move(TempBuffer));
}

void ResourceUploader::FlushAndSync(ID3D12Device* pDevice)
{
    if (mTempUploadBuffers.empty())
    {
        return;
    }

    mCopyContext->Close();
    mCopyQueue->ExecuteCommandList(mCopyContext->GetCommandList());
    mCopyQueue->Flush();

    mTempUploadBuffers.clear();
    // mCopyContext.Begin();
}
