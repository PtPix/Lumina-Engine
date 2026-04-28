#pragma once

#include "Renderer/MeshType.h"
#include "Renderer/D3D12Core/Resource/Buffer.h"

class FCommandContext;
class ResourceUploader;

class FMesh
{
public:
    FMesh() = default;
    ~FMesh() = default;

    bool Initialize(const FMeshData& MeshData, D3D12MA::Allocator* pAllocator, ResourceUploader* pUploader);

    void Draw(FCommandContext* pCommandContext);

    void Destroy();

private:
    FVertexBuffer mVertexBuffer;
    FIndexBuffer mIndexBuffer;

    uint32_t mIndexCount = 0;
};