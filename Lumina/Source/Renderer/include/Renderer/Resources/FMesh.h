#pragma once

#include "Renderer/MeshType.h"
#include "Renderer/D3D12Core/Resource/FBuffer.h"

class FCommandContext;
class FResourceUploader;

class FMesh
{
public:
    FMesh() = default;
    ~FMesh() = default;

    bool Initialize(const FMeshData& MeshData, D3D12MA::Allocator* pAllocator, FResourceUploader* pUploader);

    void Draw(FCommandContext* pCommandContext);

    void Destroy();

private:
    FVertexBuffer mVertexBuffer;
    FIndexBuffer mIndexBuffer;

    uint32_t mIndexCount = 0;
};