#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

#include "FCommandQueue.h"
#include "Renderer/D3D12Core/Resource/GpuResource.h"

class FDevice;

class FCommandContext
{
public:
    FCommandContext() = default;
    ~FCommandContext();

    FCommandContext(const FCommandContext&) = delete;
    FCommandContext& operator=(const FCommandContext&) = delete;

    bool Initialize(ID3D12Device* pDevice, ECommandQueueType Type, ID3D12CommandAllocator* pAllocator);

    void Begin();
    void Close();

    void TransitionResource(GpuResource* pResource, D3D12_RESOURCE_STATES NewState, bool bFlushImmediate = false);
    void FlushResourceBarriers();

    // Wrappers
    void SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature);
    void SetComputeRootSignature(ID3D12RootSignature* pRootSignature);
    void SetPipelineState(ID3D12PipelineState* pPipelineState);

    void SetViewport(const D3D12_VIEWPORT& Viewport);
    void SetScissorRect(const D3D12_RECT& Rect);
    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSV);
    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float Color[4]);
    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DSV, D3D12_CLEAR_FLAGS ClearFlags, float Depth, UINT8 Stencil);

    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);
    void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
    void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
    void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

    void SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps);

    void CopyBufferRegion( ID3D12Resource* pDstBuffer, UINT64 DstOffset, ID3D12Resource* pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes);

    // Getters
    ID3D12GraphicsCommandList* GetCommandList() { return mpCommandList.Get(); }
    [[nodiscard]] ECommandQueueType GetType() const { return mType; }

private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mpCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mpCommandAllocator;

    ID3D12Device* mpDevice = nullptr;
    ECommandQueueType mType = GRAPHICS;

    std::vector<D3D12_RESOURCE_BARRIER> mResourceBarriers;
};