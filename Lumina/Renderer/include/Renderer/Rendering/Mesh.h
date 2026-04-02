#pragma once
#include <d3d12.h>

class GraphicsDevice;

class FMesh
{
public:
    FMesh() = default;

    void Initialize(GraphicsDevice* pDevice, const void* Vertices, UINT VertexSize,
        UINT VertexCount, const void* Indices, UINT IndexCount);

    void Draw(ID3D12GraphicsCommandList* pCommandList) const;

private:
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};
    UINT mIndexCount = 0;
};
