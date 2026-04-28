#include "Renderer/Resources/FMesh.h"

#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

bool FMesh::Initialize(const FMeshData& MeshData, D3D12MA::Allocator* pAllocator, ResourceUploader* pUploader)
{
    mIndexCount = static_cast<uint32_t>(MeshData.Indices.size());

    const size_t VertexBufferSize = MeshData.Vertices.size() * sizeof(FStandardVertex);
    const size_t IndexBufferSize = MeshData.Indices.size() * sizeof(uint32_t);

    mVertexBuffer.Create(pAllocator, VertexBufferSize, sizeof(FStandardVertex));
    mIndexBuffer.Create(pAllocator, IndexBufferSize, DXGI_FORMAT_R32_UINT);

    pUploader->QueueUpload(&mVertexBuffer, MeshData.Vertices.data(), VertexBufferSize);
    pUploader->QueueUpload(&mIndexBuffer, MeshData.Indices.data(), IndexBufferSize);

    return true;
}

void FMesh::Draw(FCommandContext* pCommandContext)
{
    const D3D12_VERTEX_BUFFER_VIEW VertexBufferView = mVertexBuffer.GetView();
    const D3D12_INDEX_BUFFER_VIEW IndexBufferView = mIndexBuffer.GetView();

    pCommandContext->IASetVertexBuffers(0, 1, &VertexBufferView);
    pCommandContext->IASetIndexBuffer(&IndexBufferView);
    pCommandContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandContext->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
}

void FMesh::Destroy()
{
    mVertexBuffer.Destroy();
    mIndexBuffer.Destroy();
}
