#include "Renderer/D3D12Core/Resource/Texture.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Descriptors/DescriptorAllocator.h"

#include <utility>

FTexture::FTexture(FTexture&& Other) noexcept
{
    *this = std::move(Other);
}

FTexture& FTexture::operator=(FTexture&& Other) noexcept
{
    if (this != &Other)
    {
        Destroy();

        mRTV = std::move(Other.mRTV);
        mSRV = std::move(Other.mSRV);
        mDSV = std::move(Other.mDSV);

        mWidth = Other.mWidth;
        mHeight = Other.mHeight;
        mFormat = Other.mFormat;

        mpAllocation = Other.mpAllocation;
        Other.mpAllocation = nullptr;

        mpResource = std::move(Other.mpResource);
        mUsageState = Other.mUsageState;
    }
    return *this;
}
bool FTexture::Create(FDevice* pDevice, D3D12MA::Allocator* pAllocator, UINT Width, UINT Height, DXGI_FORMAT Format,
    D3D12_RESOURCE_FLAGS Flags, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pClearValue,
    const std::wstring& Name)
{
    mWidth = Width;
    mHeight = Height;
    mFormat = Format;
    mUsageState = InitialState;

    D3D12_RESOURCE_DESC ResourceDesc = {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Width = mWidth;
    ResourceDesc.Height = mHeight;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = mFormat;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    ResourceDesc.Flags = Flags;

    D3D12MA::ALLOCATION_DESC AllocDesc = {};
    AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    HRESULT hr = pAllocator->CreateResource(
        &AllocDesc,
        &ResourceDesc,
        InitialState,
        pClearValue,
        &mpAllocation,
        IID_PPV_ARGS(&mpResource)
    );

    if (FAILED(hr)) return false;

    if (!Name.empty())
    {
        mpResource->SetName(Name.c_str());
    }

    if (Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
    {
        mRTV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->Allocate(1);
        pDevice->GetDevice()->CreateRenderTargetView(mpResource.Get(), nullptr, mRTV.GetCpuHandle());
    }

    if (Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
    {
        mDSV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->Allocate(1);

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = mFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        pDevice->GetDevice()->CreateDepthStencilView(mpResource.Get(), &dsvDesc, mDSV.GetCpuHandle());
    }

    if (!(Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) && !(Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
    {
        mSRV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = mFormat;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        pDevice->GetDevice()->CreateShaderResourceView(mpResource.Get(), &srvDesc, mSRV.GetCpuHandle());
    }

    return true;
}

void FTexture::CreateFromSwapChain(FDevice* pDevice, ID3D12Resource* pResource)
{
    if (!pResource || !pDevice) return;

    mpResource = pResource;
    mUsageState = D3D12_RESOURCE_STATE_PRESENT;

    D3D12_RESOURCE_DESC ResourceDesc = mpResource->GetDesc();
    mWidth = static_cast<UINT>(ResourceDesc.Width);
    mHeight = static_cast<UINT>(ResourceDesc.Height);
    mFormat = ResourceDesc.Format;

    mRTV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->Allocate(1);
    pDevice->GetDevice()->CreateRenderTargetView(
        mpResource.Get(), nullptr, mRTV.GetCpuHandle()
        );
}

void FTexture::Create2D(FDevice* pDevice, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
    D3D12MA::Allocation* pAllocation, ID3D12Resource* pResource)
{
    mpResource = pResource;
    mpAllocation = pAllocation;
    mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

    mWidth = Width;
    mHeight = Height;
    mFormat = Format;

    mSRV = pDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(1);

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Format = mFormat;
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = 1;

    pDevice->GetDevice()->CreateShaderResourceView(mpResource.Get(), &SRVDesc, mSRV.GetCpuHandle());
}

void FTexture::Destroy()
{
    mRTV.Free();
    mSRV.Free();
    mDSV.Free();

    if (mpAllocation)
    {
        mpAllocation->Release();
        mpAllocation = nullptr;
    }

    mpResource.Reset();
}
