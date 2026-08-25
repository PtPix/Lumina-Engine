/**
 * @file D3D12CommandContext.h
 * @brief DirectX 12 Command Context Wrapper.
 *
 * Wrap ID3D12GraphicsCommandList and ID3D12CommandAllocator.
 * Provides a high-level interface for recording rendering/compute commands,
 * managing state configurations, and batching resource barriers.
 */

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

class FD3D12Device;
class D3D12GpuResource;

class FD3D12CommandContext
{
public:
    FD3D12CommandContext() = default;
    ~FD3D12CommandContext();

    FD3D12CommandContext(const FD3D12CommandContext&) = delete;
    FD3D12CommandContext& operator=(const FD3D12CommandContext&) = delete;

    // ------------------------------------------------------------------------
    // Lifecycle & Core Operations
    // ------------------------------------------------------------------------
    bool Initialize(FD3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE Type);
    void Begin();
    void Close();

    // ------------------------------------------------------------------------
    // Resource Barrier Management
    // ------------------------------------------------------------------------
    void TransitionResource(D3D12GpuResource* pResource, D3D12_RESOURCE_STATES NewState,
        uint32_t Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool bFlushImmediate = false);

    void InsertUAVBarrier(D3D12GpuResource* pResource, bool bFlushImmediate = false);

    void InsertAliasingBarrier(D3D12GpuResource* pBefore, D3D12GpuResource* pAfter, bool bFlushImmediate = false);

    void FlushResourceBarriers();

    // ------------------------------------------------------------------------
    // Pipeline State & Signatures
    // ------------------------------------------------------------------------
    void SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature) const;
    void SetComputeRootSignature(ID3D12RootSignature* pRootSignature) const;
    void SetPipelineState(ID3D12PipelineState* pPipelineState) const;

    // ------------------------------------------------------------------------
    // Root Parameters & Bindless Descriptors
    // ------------------------------------------------------------------------
    void SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) const;

    void SetGraphicsRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const;
    void SetComputeRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const;

    void SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues);
    void SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
    void SetGraphicsRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const;
    void SetGraphicsRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const;

    void SetComputeRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) const;
    void SetComputeRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const;
    void SetComputeRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const;
    void SetComputeRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const;

    // ------------------------------------------------------------------------
    // Rasterizer & Output Merger State
    // ------------------------------------------------------------------------
    void SetViewport(const D3D12_VIEWPORT& Viewport) const;
    void SetScissorRect(const D3D12_RECT& Rect) const;
    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSV) const;
    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float Color[4]);
    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DSV, D3D12_CLEAR_FLAGS ClearFlags, float Depth, UINT8 Stencil);

    // ------------------------------------------------------------------------
    // Input Assembler & Drawing / Dispatching
    // ------------------------------------------------------------------------
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology) const;
    void IASetVertexBuffers( UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) const;
    void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pViews) const;

    void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
    void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
    void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

    // ------------------------------------------------------------------------
    // Copy Operations
    // ------------------------------------------------------------------------
    void CopyBufferRegion(ID3D12Resource* pDstBuffer, UINT64 DstOffset, ID3D12Resource* pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes);

    // ------------------------------------------------------------------------
    // Debug Markers (RenderDoc / PIX)
    // ------------------------------------------------------------------------
    void BeginEvent(const char* Name) const;
    void EndEvent() const;
    void SetMarker(const char* Name) const;

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return mpCommandList.Get(); }
    [[nodiscard]] ID3D12CommandAllocator* GetCommandAllocator() const { return mpCommandAllocator.Get(); }
    [[nodiscard]] D3D12_COMMAND_LIST_TYPE GetType() const { return mType; }

private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mpCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mpCommandAllocator;

    FD3D12Device* mpDevice = nullptr;
    D3D12_COMMAND_LIST_TYPE mType = {};

    std::vector<D3D12_RESOURCE_BARRIER> mResourceBarriers;
    static constexpr uint32_t MaxBatchedBarriers = 32;
};

class FScopedGpuEvent
{
public:
    FScopedGpuEvent(FD3D12CommandContext* pContext, const char* Name) : mpContext(pContext)
    {
        if (mpContext) mpContext->BeginEvent(Name);
    }

    ~FScopedGpuEvent() { if (mpContext) mpContext->EndEvent(); }

    FScopedGpuEvent(const FScopedGpuEvent&) = delete;
    FScopedGpuEvent& operator=(const FScopedGpuEvent&) = delete;

private:
    FD3D12CommandContext* mpContext;
};

#define LUMINA_GPU_EVENT(Context, Name) FScopedGpuEvent ScopedGpuEvent_##__LINE__((Context), (Name))