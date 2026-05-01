#include <cassert>

#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"

FCommandContext::~FCommandContext()
{
    mpCommandList.Reset();
    mpCommandAllocator.Reset();
}

bool FCommandContext::Initialize(FDevice* pDevice, D3D12_COMMAND_LIST_TYPE Type)
{
    assert(pDevice != nullptr);
    mpDevice = pDevice;
    mType = Type;

    mpDevice->GetDevice()->CreateCommandAllocator(mType, IID_PPV_ARGS(&mpCommandAllocator));

    HRESULT HResult = mpDevice->GetDevice()->CreateCommandList(
        0, Type, mpCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList)
        );
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "FCommandContext::Initialize failed to create Command List.");
        return false;
    }

    mpCommandList->Close();
    mResourceBarriers.reserve(16);

    return true;
}

void FCommandContext::Begin()
{
    mpCommandAllocator.Get()->Reset();
    mpCommandList->Reset(mpCommandAllocator.Get(), nullptr);

    mResourceBarriers.clear();
}

void FCommandContext::Close()
{
    FlushResourceBarriers();
    mpCommandList->Close();
}

void FCommandContext::TransitionResource(GpuResource* pResource, D3D12_RESOURCE_STATES NewState, bool bFlushImmediate)
{
    if (!pResource || !pResource->GetResource())
    {
        return;
    }

    D3D12_RESOURCE_STATES OldState = pResource->GetUsageState();
    if (OldState == NewState)
    {
        return;
    }

    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    Barrier.Transition.pResource = pResource->GetResource();
    Barrier.Transition.StateBefore = OldState;
    Barrier.Transition.StateAfter = NewState;
    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    mResourceBarriers.push_back(Barrier);
    pResource->SetUsageState(NewState);

    if (bFlushImmediate || mResourceBarriers.size() >= 16)
    {
        FlushResourceBarriers();
    }
}

void FCommandContext::FlushResourceBarriers()
{
    if (mResourceBarriers.empty())
    {
        return;
    }

    mpCommandList->ResourceBarrier(static_cast<UINT>(mResourceBarriers.size()), mResourceBarriers.data());
    mResourceBarriers.clear();
}

void FCommandContext::SetGraphicsRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const
{
    mpCommandList->SetGraphicsRootDescriptorTable(RootIndex, BaseDescriptor);
}

void FCommandContext::SetComputeRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const
{
    mpCommandList->SetComputeRootDescriptorTable(RootIndex, BaseDescriptor);
}

void FCommandContext::SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature) const
{
    mpCommandList->SetGraphicsRootSignature(pRootSignature);
}

void FCommandContext::SetComputeRootSignature(ID3D12RootSignature* pRootSignature) const
{
    mpCommandList->SetComputeRootSignature(pRootSignature);
}

void FCommandContext::SetPipelineState(ID3D12PipelineState* pPipelineState) const
{
    mpCommandList->SetPipelineState(pPipelineState);
}

void FCommandContext::SetViewport(const D3D12_VIEWPORT& Viewport) const
{
    mpCommandList->RSSetViewports(1, &Viewport);
}

void FCommandContext::SetScissorRect(const D3D12_RECT& Rect) const
{
    mpCommandList->RSSetScissorRects(1, &Rect);
}

void FCommandContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVs,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDSV) const
{
    mpCommandList->OMSetRenderTargets(NumRTVs, pRTVs, FALSE, pDSV);
}

void FCommandContext::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float Color[4])
{
    FlushResourceBarriers();
    mpCommandList->ClearRenderTargetView(RTV, Color, 0, nullptr);
}

void FCommandContext::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DSV, D3D12_CLEAR_FLAGS ClearFlags, float Depth,
    UINT8 Stencil)
{
    FlushResourceBarriers();
    mpCommandList->ClearDepthStencilView(DSV, ClearFlags, Depth, Stencil, 0, nullptr);
}

void FCommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology) const
{
    mpCommandList->IASetPrimitiveTopology(Topology);
}

void FCommandContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation,
    UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    mpCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void FCommandContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
    INT BaseVertexLocation, UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    mpCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void FCommandContext::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    FlushResourceBarriers();
    mpCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void FCommandContext::SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) const
{
    mpCommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
}

void FCommandContext::SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet,
    const void* pSrcData, UINT DestOffsetIn32BitValues)
{
    mpCommandList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void FCommandContext::SetGraphicsRootConstantBufferView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    mpCommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
}

void FCommandContext::CopyBufferRegion(ID3D12Resource* pDstBuffer, UINT64 DstOffset, ID3D12Resource* pSrcBuffer,
                                       UINT64 SrcOffset, UINT64 NumBytes) const
{
    mpCommandList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
}

void FCommandContext::IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) const
{
    mpCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

void FCommandContext::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pViews) const
{
    mpCommandList->IASetIndexBuffer(pViews);
}
