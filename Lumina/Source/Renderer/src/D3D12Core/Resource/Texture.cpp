#include "Renderer/D3D12Core/Resource/Texture.h"

void Texture::CreateFromSwapChain(ID3D12Resource* pResource)
{
    mpResource = pResource;
    mUsageState = D3D12_RESOURCE_STATE_PRESENT;
}
