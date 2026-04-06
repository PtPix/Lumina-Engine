#pragma once

#include "ResourceView.h"
#include "D3D12MemAlloc.h"

#include <d3d12.h>

struct FTexture
{
    ID3D12Resource* Resource = nullptr;
    D3D12MA::Allocation* Allocation = nullptr;
    ResourceView SourceView;

    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    int32_t Width = 0;
    int32_t Height = 0;
    int32_t ArraySlices = 1;
    int32_t MipCount = 1;
};
