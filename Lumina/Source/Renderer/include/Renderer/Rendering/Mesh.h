#pragma once
#include <d3d12.h>

#include "Renderer/D3D12Core/Resource/Buffer.h"
#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

class GraphicsDevice;
class FDevice;

class FMesh
{
public:
    FMesh() = default;

    void Initialize(FDevice* pDevice, ResourceUploader* pUploader, const void* Vertices, UINT VertexSize,
        UINT VertexCount, const void* Indices, UINT IndexCount);

    void Draw(FCommandContext* pCommandContext) const;

private:
    VertexBuffer mVertexBuffer;
    IndexBuffer mIndexBuffer;
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};
    UINT mIndexCount = 0;
};
