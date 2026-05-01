#include "Renderer/Rendering/Mesh.h"
#include "Renderer/D3D12Core/GraphicsDevice.h"
#include "Renderer/D3D12Core/Resource/FBuffer.h"
#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

void FMesh::Initialize(FDevice* pDevice, ResourceUploader* pUploader, const void* Vertices, UINT VertexSize,
                       UINT VertexCount, const void* Indices, UINT IndexCount)
{
    mIndexCount = IndexCount;
    mVertexBuffer.Create(pDevice->GetAllocator(), VertexCount * VertexSize, VertexSize);
    mVertexBufferView = mVertexBuffer.GetView();
    mIndexBuffer.Create(pDevice->GetAllocator(), IndexCount * sizeof(uint32_t), DXGI_FORMAT_R32_UINT);
    mIndexBufferView = mIndexBuffer.GetView();
    pUploader->QueueUpload(&mVertexBuffer, Vertices, VertexCount * VertexSize);
    pUploader->QueueUpload(&mIndexBuffer, Indices, IndexCount * sizeof(uint32_t));
}

void FMesh::Draw(FCommandContext* pCommandContext) const
{
    ID3D12GraphicsCommandList* pCommandList = pCommandContext->GetCommandList();
    pCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
    pCommandList->IASetIndexBuffer(&mIndexBufferView);

    // 🔴 最致命的一步：必须调用 Context 的 Draw，触发 Commit 拦截！
    pCommandContext->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
    // pCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
    // pCommandList->IASetIndexBuffer(&mIndexBufferView);
    // pCommandList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
}
