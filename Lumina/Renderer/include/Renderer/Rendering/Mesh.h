#pragma once
#include <d3d12.h>

#include "Renderer/D3D12Core/GraphicsDevice.h"

class FMesh
{
public:
    FMesh() = default;

    void Initialize(GraphicsDevice* pDevice, const void* Vertices, UINT VertexSize,
        UINT VertexCount, const void* Indices, UINT IndexCount)
    {
        mIndexCount = IndexCount;
        pDevice->GetVertexBufferHeap().AllocVertexBuffer(VertexCount, VertexSize, Vertices, &mVertexBufferView);
        pDevice->GetIndexBufferHeap().AllocIndexBuffer(IndexCount, sizeof(uint32_t), Indices, &mIndexBufferView);
    }

    void Draw(ID3D12GraphicsCommandList* pCommandList) const
    {
        pCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
        pCommandList->IASetIndexBuffer(&mIndexBufferView);
        pCommandList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
    }

private:
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};
    UINT mIndexCount = 0;
};
