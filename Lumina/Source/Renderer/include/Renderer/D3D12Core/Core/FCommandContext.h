#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

#include "Renderer/D3D12Core/Resource/GpuResource.h"

class FDevice;

class FCommandContext
{
public:
    FCommandContext() = default;
    ~FCommandContext();
    FCommandContext(const FCommandContext&) = delete;
    FCommandContext& operator=(const FCommandContext&) = delete;

    // Basic Operations
    bool Initialize(FDevice* pDevice, D3D12_COMMAND_LIST_TYPE Type);

    void Begin();
    void Close();

    // Barrier Operations
    void TransitionResource(GpuResource* pResource, D3D12_RESOURCE_STATES NewState, bool bFlushImmediate = false);
    void FlushResourceBarriers();

    // Bind Bindless Descriptor Table
    void SetGraphicsRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const;
    void SetComputeRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const;

    // Wrappers
    void SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature) const;
    void SetComputeRootSignature(ID3D12RootSignature* pRootSignature) const;
    void SetPipelineState(ID3D12PipelineState* pPipelineState) const;

    void SetViewport(const D3D12_VIEWPORT& Viewport) const;
    void SetScissorRect(const D3D12_RECT& Rect) const;
    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSV) const;
    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float Color[4]);
    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DSV, D3D12_CLEAR_FLAGS ClearFlags, float Depth, UINT8 Stencil);
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology) const;

    void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
    void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
    void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

    void SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) const;
    void SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues);
    void SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void CopyBufferRegion( ID3D12Resource* pDstBuffer, UINT64 DstOffset, ID3D12Resource* pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes) const;

    void IASetVertexBuffers( UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) const;
    void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pViews) const;

    // Getters
    [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return mpCommandList.Get(); }
    [[nodiscard]] ID3D12CommandAllocator* GetCommandAllocator() const { return mpCommandAllocator.Get(); }
    [[nodiscard]] D3D12_COMMAND_LIST_TYPE GetType() const { return mType; }

private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mpCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mpCommandAllocator;

    FDevice* mpDevice = nullptr;
    D3D12_COMMAND_LIST_TYPE mType;

    std::vector<D3D12_RESOURCE_BARRIER> mResourceBarriers;
};