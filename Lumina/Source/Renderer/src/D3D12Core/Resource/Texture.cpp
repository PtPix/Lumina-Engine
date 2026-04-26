#include "Renderer/D3D12Core/Resource/Texture.h"

#include "Renderer/D3D12Core/Core/FDevice.h"

void Texture::CreateFromSwapChain(FDevice* pDevice, ID3D12Resource* pResource)
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

void Texture::Create2D(FDevice* pDevice, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
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

    // 直接使用刚分配的 CPU 句柄原件！
    pDevice->GetDevice()->CreateShaderResourceView(mpResource.Get(), &SRVDesc, mSRV.GetCpuHandle());
}

void Texture::Destroy()
{
    if (mpAllocation)
    {
        mpAllocation->Release();
        mpAllocation = nullptr;
    }
}
