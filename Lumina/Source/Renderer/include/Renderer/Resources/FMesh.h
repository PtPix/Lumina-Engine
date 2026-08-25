#pragma once

#include "Renderer/MeshType.h"
#include "../D3D12/D3D12Buffer.h"

class FD3D12CommandContext;
class FD3D12ResourceUploader;

class FMesh
{
public:
    FMesh() = default;
    ~FMesh() { Destroy(); }

    bool Initialize(const FMeshData& MeshData, D3D12MA::Allocator* pAllocator, FD3D12ResourceUploader* pUploader);

    void Draw(FD3D12CommandContext* pCommandContext);

    void Destroy();

private:
    FD3D12VertexBuffer mVertexBuffer;
    FD3D12IndexBuffer mIndexBuffer;

    uint32_t mIndexCount = 0;
};