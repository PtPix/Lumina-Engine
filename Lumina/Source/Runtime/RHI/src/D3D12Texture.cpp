#include "D3D12Texture.h"
#include "D3D12FormatUtils.h"
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12DeferredReleaseQueue.h"
#include "D3D12DescriptorAllocator.h"
#include "D3D12BindlessDescriptorHeap.h"
#include "D3D12Common.h"
#include "Logger/Logger.h"

#include <utility>

FD3D12Texture::FD3D12Texture(FD3D12Texture&& Other) noexcept
{
    *this = std::move(Other);
}

FD3D12Texture& FD3D12Texture::operator=(FD3D12Texture&& Other) noexcept
{
    if (this != &Other)
    {
        Destroy();

        mDesc = std::move(Other.mDesc);
        mNumMips = Other.mNumMips;
        mArraySize = Other.mArraySize;
        mResourceFormat = Other.mResourceFormat;

        mRTVs = std::move(Other.mRTVs);
        mDSVs = std::move(Other.mDSVs);
        mSRV = std::move(Other.mSRV);
        mUAVs = std::move(Other.mUAVs);

        mBindlessSRVIndex = Other.mBindlessSRVIndex;
        mBindlessUAVIndices = std::move(Other.mBindlessUAVIndices);

        mpAllocation = std::move(Other.mpAllocation);
        mpDevice = Other.mpDevice;

        mbExternalResource = Other.mbExternalResource;

        mpResource = std::move(Other.mpResource);
        mAllSubresourcesState = Other.mAllSubresourcesState;
        mbAllSubresourcesSame = Other.mbAllSubresourcesSame;
        mNumSubresources = Other.mNumSubresources;
        mSubresourceStates = std::move(Other.mSubresourceStates);

        Other.mBindlessSRVIndex = InvalidBindlessIndex;
        Other.mBindlessUAVIndices.clear();
        Other.mpDevice = nullptr;
        Other.mNumMips = 1;
        Other.mArraySize = 1;
        Other.mbExternalResource = false;
    }
    return *this;
}

bool FD3D12Texture::Create(FD3D12Device *pDevice, D3D12MA::Allocator *pAllocator, const FD3D12TextureDesc &Desc)
{
    if (!pDevice || !pAllocator) return false;

    Destroy();

    mpDevice = pDevice;
    mDesc = Desc;
    mbExternalResource = false;

    // Mip Num
    mNumMips = (mDesc.MipLevels == 0)
        ? D3D12FormatUtils::CalculateNumMips(mDesc.Width, mDesc.Height, mDesc.Dimension == ED3D12TextureDimension::Texture3D ? mDesc.DepthOrArraySize : 1)
        : mDesc.DepthOrArraySize;

    // Array Size
    mArraySize = (mDesc.Dimension == ED3D12TextureDimension::Texture3D) ? 1 : mDesc.DepthOrArraySize;
    if (mDesc.Dimension == ED3D12TextureDimension::TextureCube && mArraySize % 6 != 0)
    {
        LUMINA_LOG_ERROR(RHI, "FTexture::Create: cube 纹理的 DepthOrArraySize 必须是 6 的倍数");
        return false;
    }

    mResourceFormat = D3D12FormatUtils::GetResourceFormat(mDesc.Format);

    // Flags
    D3D12_RESOURCE_FLAGS ResourceFlags = D3D12_RESOURCE_FLAG_NONE;
    if (HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowRenderTarget))
        ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowDepthStencil))
        ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowUnorderedAccess))
        ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (HasFlag(mDesc.Flags, ED3D12TextureFlags::DenyShaderResource))
        ResourceFlags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

    // Resource Desc
    D3D12_RESOURCE_DESC ResourceDesc = {};
    ResourceDesc.Dimension = (mDesc.Dimension == ED3D12TextureDimension::Texture3D)
        ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Width = mDesc.Width;
    ResourceDesc.Height = mDesc.Height;
    ResourceDesc.DepthOrArraySize = static_cast<UINT16>(mDesc.DepthOrArraySize);
    ResourceDesc.MipLevels = static_cast<UINT16>(mNumMips);
    ResourceDesc.Format = mResourceFormat;
    ResourceDesc.SampleDesc.Count = mDesc.SampleCount;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    ResourceDesc.Flags = ResourceFlags;

    // Clear Value
    D3D12_CLEAR_VALUE ClearValue = mDesc.ClearValue;
    const D3D12_CLEAR_VALUE* pClearValue = nullptr;
    if (mDesc.bUseClearValue && (
        HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowRenderTarget) || HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowDepthStencil)))
    {
        ClearValue.Format = HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowDepthStencil)
                                ? D3D12FormatUtils::GetDSVFormat(mResourceFormat)
                                : mDesc.Format;
        pClearValue = &ClearValue;
    }

    D3D12MA::ALLOCATION_DESC AllocDesc = {};
    AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    D3D12MA::Allocation* pRawAllocation = nullptr;
    HRESULT HResult = pAllocator->CreateResource(&AllocDesc, &ResourceDesc, mDesc.InitialState, pClearValue, &pRawAllocation, IID_PPV_ARGS(&mpResource));

    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "FTexture::Create Failed: %ux%u fmt=%d",
                   mDesc.Width, mDesc.Height, static_cast<int>(mDesc.Format));
        return false;
    }
    mpAllocation.Attach(pRawAllocation);

    if (!mDesc.DebugName.empty())
    {
        mpResource->SetName(mDesc.DebugName.c_str());
    }

    // State Track
    const uint32_t NumSubResources =
        (mDesc.Dimension == ED3D12TextureDimension::Texture3D) ? mNumMips : (mNumMips * mArraySize);
    InitStateTracking(NumSubResources, mDesc.InitialState);

    CreateViews(pDevice);

    return true;
}

void FD3D12Texture::CreateFromSwapChain(FD3D12Device* pDevice, ID3D12Resource* pResource)
{
    if (!pResource || !pDevice) return;

    Destroy();

    mpDevice = pDevice;
    mpResource.Attach(pResource);
    pResource->AddRef();

    D3D12_RESOURCE_DESC ResourceDesc = mpResource->GetDesc();

    mDesc = {};
    mDesc.Dimension = ED3D12TextureDimension::Texture2D;
    mDesc.Width = static_cast<uint32_t>(ResourceDesc.Width);
    mDesc.Height = ResourceDesc.Height;
    mDesc.DepthOrArraySize = 1;
    mDesc.MipLevels = 1;
    mDesc.Format = ResourceDesc.Format;
    mDesc.Flags = ED3D12TextureFlags::AllowRenderTarget;
    mDesc.InitialState = D3D12_RESOURCE_STATE_PRESENT;
    mDesc.DebugName = L"SwapChainBackBuffer";

    mResourceFormat = ResourceDesc.Format;
    mNumMips = 1;
    mArraySize = 1;

    InitStateTracking(1, D3D12_RESOURCE_STATE_PRESENT);

    mbExternalResource = true;

    // BackBuffer only needs RTV
    mRTVs.resize(1);
    mRTVs[0] = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->Allocate(1);
    pDevice->GetDevice()->CreateRenderTargetView(mpResource.Get(), nullptr, mRTVs[0].GetCpuHandle());
}

void FD3D12Texture::CreateViews(FD3D12Device *pDevice)
{
    if (HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowRenderTarget)) CreateRTVsInternal(pDevice);
    if (HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowDepthStencil)) CreateDSVsInternal(pDevice);

    // Depth Texture will create SRV too unless it is denied.
    if (!HasFlag(mDesc.Flags, ED3D12TextureFlags::DenyShaderResource)) CreateSRVInternal(pDevice);
    if (HasFlag(mDesc.Flags, ED3D12TextureFlags::AllowUnorderedAccess)) CreateUAVsInternal(pDevice);
}

void FD3D12Texture::CreateSRVInternal(FD3D12Device *pDevice)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Format = D3D12FormatUtils::GetSRVFormat(mResourceFormat);

    switch (mDesc.Dimension)
    {
        case ED3D12TextureDimension::Texture2D:
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MostDetailedMip = 0;
            SRVDesc.Texture2D.MipLevels = mNumMips;
            SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            break;

        case ED3D12TextureDimension::Texture2DArray:
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            SRVDesc.Texture2DArray.MostDetailedMip = 0;
            SRVDesc.Texture2DArray.MipLevels = mNumMips;
            SRVDesc.Texture2DArray.FirstArraySlice = 0;
            SRVDesc.Texture2DArray.ArraySize = mArraySize;
            SRVDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            break;

        case ED3D12TextureDimension::Texture3D:
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            SRVDesc.Texture3D.MostDetailedMip = 0;
            SRVDesc.Texture3D.MipLevels = mNumMips;
            SRVDesc.Texture3D.ResourceMinLODClamp = 0.0f;
            break;

        case ED3D12TextureDimension::TextureCube:
            if (mArraySize > 6)
            {
                SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                SRVDesc.TextureCubeArray.MostDetailedMip = 0;
                SRVDesc.TextureCubeArray.MipLevels = mNumMips;
                SRVDesc.TextureCubeArray.First2DArrayFace = 0;
                SRVDesc.TextureCubeArray.NumCubes = mArraySize / 6;
                SRVDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                SRVDesc.TextureCube.MostDetailedMip = 0;
                SRVDesc.TextureCube.MipLevels = mNumMips;
                SRVDesc.TextureCube.ResourceMinLODClamp = 0.0f;
            }
            break;
    }

    mSRV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);
    pDevice->GetDevice()->CreateShaderResourceView(mpResource.Get(), &SRVDesc, mSRV.GetCpuHandle());

    if (FD3D12BindlessDescriptorHeap* pHeap = pDevice->GetBindlessDescriptorHeap())
    {
        mBindlessSRVIndex = pHeap->AllocateSlot();
        pHeap->CopyDescriptor(pDevice, mSRV.GetCpuHandle(), mBindlessSRVIndex);
    }
}

void FD3D12Texture::CreateRTVsInternal(FD3D12Device *pDevice)
{
    const uint32_t NumSlices = (mDesc.Dimension == ED3D12TextureDimension::Texture3D) ? 1 : mArraySize;
    mRTVs.resize(mNumMips * NumSlices);

    for (uint32_t Slice = 0; Slice < NumSlices; Slice++)
    {
        for (uint32_t Mip = 0; Mip < mNumMips; Mip++)
        {
            D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
            RTVDesc.Format = D3D12FormatUtils::GetRTVFormat(mDesc.Format);

            if (mDesc.Dimension == ED3D12TextureDimension::Texture3D)
            {
                RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                RTVDesc.Texture3D.MipSlice = Mip;
                RTVDesc.Texture3D.FirstWSlice = 0;
                RTVDesc.Texture3D.WSize = static_cast<UINT>(-1);
            }
            else if (mDesc.Dimension == ED3D12TextureDimension::Texture2D && mArraySize == 1)
            {
                RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                RTVDesc.Texture2D.MipSlice = Mip;
                RTVDesc.Texture2D.PlaneSlice = 0;
            }
            else
            {
                // Array / Cube : Per Slice and Mip a RTV
                RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                RTVDesc.Texture2DArray.MipSlice = Mip;
                RTVDesc.Texture2DArray.FirstArraySlice = Slice;
                RTVDesc.Texture2DArray.ArraySize = 1;
                RTVDesc.Texture2DArray.PlaneSlice = 0;
            }

            const uint32_t Index = Mip + Slice * mNumMips;
            mRTVs[Index] = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->Allocate(1);
            pDevice->GetDevice()->CreateRenderTargetView(mpResource.Get(), &RTVDesc, mRTVs[Index].GetCpuHandle());
        }
    }
}

void FD3D12Texture::CreateDSVsInternal(FD3D12Device *pDevice)
{
    mDSVs.resize(mNumMips * mArraySize);

    for (uint32_t Slice = 0; Slice < mArraySize; Slice++)
    {
        for (uint32_t Mip = 0; Mip < mNumMips; Mip++)
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
            DSVDesc.Format = D3D12FormatUtils::GetDSVFormat(mResourceFormat);
            DSVDesc.Flags = D3D12_DSV_FLAG_NONE;

            if (mArraySize == 1)
            {
                DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                DSVDesc.Texture2D.MipSlice = Mip;
            }
            else
            {
                DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                DSVDesc.Texture2DArray.MipSlice = Mip;
                DSVDesc.Texture2DArray.FirstArraySlice = Slice;
                DSVDesc.Texture2DArray.ArraySize = 1;
            }

            const uint32_t Index = Mip + Slice * mNumMips;
            mDSVs[Index] = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->Allocate(1);
            pDevice->GetDevice()->CreateDepthStencilView(mpResource.Get(), &DSVDesc, mDSVs[Index].GetCpuHandle());
        }
    }
}

void FD3D12Texture::CreateUAVsInternal(FD3D12Device *pDevice)
{
    mUAVs.resize(mNumMips);
    mBindlessUAVIndices.assign(mNumMips, InvalidBindlessIndex);

    FD3D12BindlessDescriptorHeap* pHeap = pDevice->GetBindlessDescriptorHeap();

    for (uint32_t Mip = 0; Mip < mNumMips; Mip++)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.Format = D3D12FormatUtils::GetUAVFormat(mDesc.Format);

        switch (mDesc.Dimension)
        {
            case ED3D12TextureDimension::Texture3D:
                UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                UAVDesc.Texture3D.MipSlice = Mip;
                UAVDesc.Texture3D.FirstWSlice = 0;
                UAVDesc.Texture3D.WSize = (std::max)(1u, mDesc.DepthOrArraySize >> Mip);
                break;

            case ED3D12TextureDimension::Texture2D:
                if (mArraySize == 1)
                {
                    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                    UAVDesc.Texture2D.MipSlice = Mip;
                    UAVDesc.Texture2D.PlaneSlice = 0;
                    break;
                }
                [[fallthrough]];

            case ED3D12TextureDimension::Texture2DArray:
            case ED3D12TextureDimension::TextureCube:
                UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                UAVDesc.Texture2DArray.MipSlice = Mip;
                UAVDesc.Texture2DArray.FirstArraySlice = 0;
                UAVDesc.Texture2DArray.ArraySize = mArraySize;
                UAVDesc.Texture2DArray.PlaneSlice = 0;
                break;
        }

        mUAVs[Mip] = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);

        pDevice->GetDevice()->CreateUnorderedAccessView(mpResource.Get(), nullptr, &UAVDesc, mUAVs[Mip].GetCpuHandle());

        if (pHeap)
        {
            mBindlessUAVIndices[Mip] = pHeap->AllocateSlot();
            pHeap->CopyDescriptor(pDevice, mUAVs[Mip].GetCpuHandle(), mBindlessUAVIndices[Mip]);
        }
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12Texture::GetRTV(uint32_t Mip, uint32_t Slice) const
{
    const uint32_t Index = Mip + Slice * mNumMips;
    return (Index < mRTVs.size() && mRTVs[Index].IsValid()) ? mRTVs[Index].GetCpuHandle() : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12Texture::GetDSV(uint32_t Mip, uint32_t Slice) const
{
    const uint32_t Index = Mip + Slice * mNumMips;
    return (Index < mDSVs.size() && mDSVs[Index].IsValid()) ? mDSVs[Index].GetCpuHandle() : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

D3D12_CPU_DESCRIPTOR_HANDLE FD3D12Texture::GetUAV(uint32_t Mip) const
{
    return (Mip < mUAVs.size() && mUAVs[Mip].IsValid()) ? mUAVs[Mip].GetCpuHandle() : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

void FD3D12Texture::ReleaseBindlessSlots()
{
    if (!mpDevice) return;
    FD3D12BindlessDescriptorHeap* pHeap = mpDevice->GetBindlessDescriptorHeap();
    if (!pHeap) return;

    FD3D12CommandQueue* pQueue = mpDevice->GetGraphicsCommandQueue();
    const uint64_t Fence = pQueue ? pQueue->GetNextFenceValue() : 0;

    if (mBindlessSRVIndex != InvalidBindlessIndex)
    {
        pHeap->FreeSlot(mBindlessSRVIndex, pQueue, Fence);
        mBindlessSRVIndex = InvalidBindlessIndex;
    }
    for (uint32_t Index : mBindlessUAVIndices)
    {
        if (Index != InvalidBindlessIndex) pHeap->FreeSlot(Index, pQueue, Fence);
    }
    mBindlessUAVIndices.clear();
}

void FD3D12Texture::Destroy()
{
    ReleaseBindlessSlots();

    mRTVs.clear();
    mDSVs.clear();
    mUAVs.clear();
    mSRV.Free();

    if (!mbExternalResource && (mpResource || mpAllocation))
    {
        auto pResource = mpResource;
        auto pAllocation = mpAllocation;

        FD3D12DeferredReleaseQueue::Enqueue([pResource, pAllocation]() mutable
        {
            pAllocation.Reset();
            pResource.Reset();
        });
    }

    mpResource.Reset();
    mpAllocation.Reset();
    mpDevice = nullptr;

    mNumMips = 1;
    mArraySize = 1;
    InitStateTracking(1, D3D12_RESOURCE_STATE_COMMON);
}
