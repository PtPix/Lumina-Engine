#pragma once

#include "GpuResource.h"

class Texture : public GpuResource
{
public:
    void CreateFromSwapChain(ID3D12Resource* pResource);
};