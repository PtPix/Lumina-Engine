#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

#include "FCommandQueue.h"
#include "Renderer/D3D12Core/Descriptors/FDynamicDescriptorHeap.h"
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

    // Bind Descriptor
    inline void SetGraphicsRootDescriptorTable(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, UINT NumDescriptors = 1)
    {
        mDynamicDescriptorHeap->StageDescriptors(RootIndex, Offset, NumDescriptors, CpuHandle);
    }
    inline void SetComputeRootDescriptorTable(UINT RootIndex, D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, UINT NumDescriptors = 1)
    {
        mDynamicDescriptorHeap->StageDescriptors(RootIndex, 0, NumDescriptors, CpuHandle);
    }

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

    void CleanupDynamicHeaps(uint64_t FenceValue);

    // Getters
    ID3D12GraphicsCommandList* GetCommandList() { return mpCommandList.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() { return mpCommandAllocator.Get(); }
    [[nodiscard]] D3D12_COMMAND_LIST_TYPE GetType() const { return mType; }

private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mpCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mpCommandAllocator;

    FDevice* mpDevice = nullptr;
    D3D12_COMMAND_LIST_TYPE mType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    std::vector<D3D12_RESOURCE_BARRIER> mResourceBarriers;

    std::unique_ptr<FDynamicDescriptorHeap> mDynamicDescriptorHeap;
};