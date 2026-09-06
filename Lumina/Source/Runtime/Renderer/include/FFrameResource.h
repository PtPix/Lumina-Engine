#pragma once

#include <d3d12.h>
#include "D3D12Buffer.h"
#include "SharedTypes.h"

class FFrameResource
{
public:
    FFrameResource() = default;
    ~FFrameResource()
    {
        GlobalPassBuffer.Destroy();
        InstanceBuffer.Destroy();
        MaterialBuffer.Destroy();
    }

    void Initialize(D3D12MA::Allocator* pAllocator, uint32_t MaxInstances, uint32_t MaxMaterials)
    {
        // TODO: Define proper global pass data structure
        GlobalPassBuffer.Create(pAllocator, 256, L"Frame_GlobalPass");
        InstanceBuffer.Create(pAllocator, sizeof(FInstanceData) * MaxInstances, L"Frame_InstanceBuffer");
        MaterialBuffer.Create(pAllocator, sizeof(FPBRMaterialData) * MaxMaterials, L"Frame_MaterialBuffer");
    }

    FD3D12ConstantBuffer GlobalPassBuffer;
    FD3D12UploadBuffer InstanceBuffer;
    FD3D12UploadBuffer MaterialBuffer;
};