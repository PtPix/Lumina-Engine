#include "Renderer/Rendering/Mesh.h"
#include "Renderer/D3D12Core/GraphicsDevice.h"

void FMesh::Initialize(GraphicsDevice* pDevice, const void* Vertices, UINT VertexSize,
                       UINT VertexCount, const void* Indices, UINT IndexCount)
{
    mIndexCount = IndexCount;
    pDevice->GetVertexBufferHeap().AllocVertexBuffer(VertexCount, VertexSize, Vertices, &mVertexBufferView);
    pDevice->GetIndexBufferHeap().AllocIndexBuffer(IndexCount, sizeof(uint32_t), Indices, &mIndexBufferView);
}

void FMesh::Draw(ID3D12GraphicsCommandList* pCommandList) const
{
    pCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
    pCommandList->IASetIndexBuffer(&mIndexBufferView);
    pCommandList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
}
