#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

#include "Renderer/D3D12Core/Core/FDevice.h"

void ResourceUploader::Initialize(D3D12MA::Allocator* pAllocator, FCommandContext* pCopyContext, FCommandQueue* pCopyQueue)
{
    mpAllocator = pAllocator;
    mCopyContext = pCopyContext;
    mCopyQueue = pCopyQueue;
    mCopyContext->Begin();
}

void ResourceUploader::QueueUpload(FBuffer* pDestBuffer, const void* pData, size_t DataSize)
{
    FUploadBuffer TempBuffer;
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

void ResourceUploader::UploadTexture(FDevice* pDevice, FTexture* pDestTexture, const void* pData, uint32_t Width,
    uint32_t Height, uint32_t BytesPerPixel)
{
    ID3D12Resource* pDestResource = pDestTexture->GetResource();
    D3D12_RESOURCE_DESC Desc = pDestResource->GetDesc();

    // 1. 获取显存脚印 (Footprint)
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
    UINT NumRows; UINT64 RowSizeInBytes; UINT64 TotalBytes;
    pDevice->GetDevice()->GetCopyableFootprints(&Desc, 0, 1, 0, &Footprint, &NumRows, &RowSizeInBytes, &TotalBytes);

    FUploadBuffer TempBuffer;
    TempBuffer.Create(mpAllocator, TotalBytes, L"TempUpload_Texture");
    // // 2. 利用 D3D12MA 创建一个临时的 UploadBuffer
    // D3D12MA::ALLOCATION_DESC AllocDesc = {};
    // AllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    //
    // D3D12_RESOURCE_DESC BufferDesc = {};
    // BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    // BufferDesc.Width = TotalBytes;
    // BufferDesc.Height = 1; BufferDesc.DepthOrArraySize = 1; BufferDesc.MipLevels = 1;
    // BufferDesc.SampleDesc.Count = 1; BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    // BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    //
    // Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer;
    // D3D12MA::Allocation* pUploadAllocation = nullptr;
    // mpAllocator->CreateResource(&AllocDesc, &BufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, &pUploadAllocation, IID_PPV_ARGS(&pUploadBuffer));

    // 3. 内存拷贝
    uint8_t* pMappedData = static_cast<uint8_t*>(TempBuffer.Map());
    // pUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData));
    const uint8_t* pSourceData = static_cast<const uint8_t*>(pData);
    uint32_t SourceRowPitch = Width * BytesPerPixel;
    for (UINT y = 0; y < NumRows; y++)
    {
        memcpy(pMappedData + y * Footprint.Footprint.RowPitch, pSourceData + y * SourceRowPitch, SourceRowPitch);
    }
    TempBuffer.Unmap();

    // 4. 录制 CopyTextureRegion
    D3D12_TEXTURE_COPY_LOCATION DestLocation = {};
    DestLocation.pResource = pDestResource;
    DestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    DestLocation.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION SourceLocation = {};
    SourceLocation.pResource = TempBuffer.GetResource();
    SourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    SourceLocation.PlacedFootprint = Footprint;

    // 假设你的 Uploader 持有一个 FCommandContext 指针 mpCopyContext
    mCopyContext->GetCommandList()->CopyTextureRegion(&DestLocation, 0, 0, 0, &SourceLocation, nullptr);

    // 5. 转换状态为 Shader 可读！
    mCopyContext->TransitionResource(pDestTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // 6. 将这个临时的 Allocation 存起来，等到 FlushAndSync 的时候再释放
    mTempUploadBuffers.push_back(std::move(TempBuffer));
}

void ResourceUploader::FlushAndSync()
{
    if (mTempUploadBuffers.empty())
    {
        return;
    }

    mCopyContext->Close();
    mCopyQueue->ExecuteCommandList(mCopyContext->GetCommandList());
    mCopyQueue->Flush();

    mTempUploadBuffers.clear();
    // mCopyContext->Begin();
}
