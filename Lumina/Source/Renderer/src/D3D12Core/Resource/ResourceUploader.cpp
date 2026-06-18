#include "Renderer/D3D12Core/Resource/ResourceUploader.h"
#include "Renderer/D3D12Core/Resource/Texture.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Core/CommandQueue.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"

#include <cassert>

void FResourceUploader::Initialize(FDevice* pDevice)
{
    mpDevice = pDevice;
    // TODO : Change to Copy Queue
    mpCommandQueue = mpDevice->GetGraphicsCommandQueue();
}

void FResourceUploader::QueueUpload(FBuffer* pDestBuffer, const void* pData, size_t DataSize)
{
    assert(mpCurrentContext != nullptr);

    FUploadBuffer TempBuffer;
    TempBuffer.Create(mpDevice->GetAllocator(), DataSize, L"TempUpload");

    void* pMapped = TempBuffer.Map();
    memcpy(pMapped, pData, DataSize);
    TempBuffer.Unmap();

    mpCurrentContext->TransitionResource(pDestBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    mpCurrentContext->FlushResourceBarriers();

    mpCurrentContext->CopyBufferRegion(pDestBuffer->GetResource(), 0, TempBuffer.GetResource(), 0, DataSize);

    mpCurrentContext->TransitionResource(pDestBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

    mCurrentTempUploadBuffers.push_back(std::move(TempBuffer));
}

void FResourceUploader::UploadTexture(FTexture* pDestTexture, const void* pData, uint32_t Width,
                                      uint32_t Height, uint32_t BytesPerPixel)
{
    assert(pDestTexture != nullptr && pDestTexture->GetResource() != nullptr);

    if (mpCurrentContext == nullptr)
    {
        mpCurrentContext = mpCommandQueue->AllocateContext();
    }

    ID3D12Resource* pDestResource = pDestTexture->GetResource();
    D3D12_RESOURCE_DESC Desc = pDestResource->GetDesc();

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
    UINT NumRows; UINT64 RowSizeInBytes; UINT64 TotalBytes;
    mpDevice->GetDevice()->GetCopyableFootprints(&Desc, 0, 1, 0, &Footprint, &NumRows, &RowSizeInBytes, &TotalBytes);

    FUploadBuffer TempBuffer;
    TempBuffer.Create(mpDevice->GetAllocator(), TotalBytes, L"TempUpload_Texture");

    auto* pMappedData = static_cast<uint8_t*>(TempBuffer.Map());
    const auto* pSourceData = static_cast<const uint8_t*>(pData);

    // NOTE: This assumes an uncompressed format (e.g. RGBA8).
    // Block-compressed formats (DXT/BC) will require different pitch calculations.
    uint32_t SourceRowPitch = Width * BytesPerPixel;
    for (UINT y = 0; y < NumRows; y++)
    {
        memcpy(pMappedData + y * Footprint.Footprint.RowPitch, pSourceData + y * SourceRowPitch, SourceRowPitch);
    }
    TempBuffer.Unmap();

    // Start Copy
    mpCurrentContext->TransitionResource(pDestTexture, D3D12_RESOURCE_STATE_COPY_DEST);
    mpCurrentContext->FlushResourceBarriers();

    D3D12_TEXTURE_COPY_LOCATION DestLocation = {};
    DestLocation.pResource = pDestResource;
    DestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    DestLocation.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION SourceLocation = {};
    SourceLocation.pResource = TempBuffer.GetResource();
    SourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    SourceLocation.PlacedFootprint = Footprint;

    mpCurrentContext->GetCommandList()->CopyTextureRegion(&DestLocation, 0, 0, 0, &SourceLocation, nullptr);

    if (mpCurrentContext->GetType() == D3D12_COMMAND_LIST_TYPE_COPY)
    {
        mpCurrentContext->TransitionResource(pDestTexture, D3D12_RESOURCE_STATE_COMMON);
    }
    else
    {
        mpCurrentContext->TransitionResource(pDestTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    mCurrentTempUploadBuffers.push_back(std::move(TempBuffer));
}

uint64_t FResourceUploader::SubmitPendingUploads()
{
    if (mpCurrentContext == nullptr)
        return 0;

    uint64_t FenceValue = mpCommandQueue->ExecuteCommandContext(mpCurrentContext);
    mpCurrentContext = nullptr;

    mInFlightUploads.push({ FenceValue, std::move(mCurrentTempUploadBuffers) });

    mCurrentTempUploadBuffers.clear();

    return FenceValue;
}

void FResourceUploader::CleanUpStaleUploads()
{
    while (!mInFlightUploads.empty())
    {
        if (mpCommandQueue->IsFenceComplete(mInFlightUploads.front().FenceValue))
        {
            mInFlightUploads.pop();
        }
        else
        {
            break;
        }
    }
}

void FResourceUploader::FlushAndSync()
{
    SubmitPendingUploads();
    // if (mpCurrentContext)
    // {
    //     EndUpLoadAndExecute();
    // }

    mpCommandQueue->Flush();
    CleanUpStaleUploads();
}
