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
