#pragma once

#include <d3d12.h>

#include "Renderer/RenderTypes.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Resource/Buffer.h"
#include "Renderer/Scene/FSceneView.h"

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
        GlobalPassBuffer.Create(pAllocator, sizeof(FGlobalPassData), 1,
                                D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, L"Frame_GlobalPass");

        InstanceBuffer.Create(pAllocator, sizeof(FInstanceData), MaxInstances,
                              D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, L"Frame_InstanceBuffer");

        MaterialBuffer.Create(pAllocator, sizeof(FPBRMaterialData), MaxMaterials,
                              D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, L"Frame_MaterialBuffer");
    }

    FBuffer GlobalPassBuffer;
    FBuffer InstanceBuffer;
    FBuffer MaterialBuffer;
};
