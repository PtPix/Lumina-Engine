#include "Renderer/D3D12/D3D12CommandContext.h"
#include "Renderer/D3D12/D3D12Device.h"
#include "Renderer/D3D12/D3D12GpuResource.h"
#include "Renderer/D3D12/D3D12Common.h"

#include <cassert>

FD3D12CommandContext::~FD3D12CommandContext()
{
    mpCommandList.Reset();
    mpCommandAllocator.Reset();
}

bool FD3D12CommandContext::Initialize(FD3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE Type)
{
    assert(pDevice != nullptr);
    mpDevice = pDevice;
    mType = Type;

    HRESULT HResult = mpDevice->GetDevice()->CreateCommandAllocator(mType, IID_PPV_ARGS(&mpCommandAllocator));
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "FCommandContext::Initialize failed to create Command Allocator.");
        return false;
    }

    HResult = mpDevice->GetDevice()->CreateCommandList(
        0, Type, mpCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList)
        );
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "FCommandContext::Initialize failed to create Command List.");
        return false;
    }

    mpCommandList->Close();
    mResourceBarriers.reserve(MaxBatchedBarriers);

    return true;
}

void FD3D12CommandContext::Begin()
{
    mpCommandAllocator->Reset();
    mpCommandList->Reset(mpCommandAllocator.Get(), nullptr);

    mResourceBarriers.clear();
}

void FD3D12CommandContext::Close()
{
    FlushResourceBarriers();
    mpCommandList->Close();
}

void FD3D12CommandContext::TransitionResource(D3D12GpuResource *pResource, D3D12_RESOURCE_STATES NewState, uint32_t Subresource,
    bool bFlushImmediate)
{
    if (!pResource || !pResource->GetResource()) return;

    const bool bAllSubresources = (Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    Barrier.Transition.pResource = pResource->GetResource();
    Barrier.Transition.StateAfter = NewState;

    if (bAllSubresources)
    {
        if (pResource->AreAllSubresourcesSame())
        {
            const D3D12_RESOURCE_STATES OldState = pResource->GetUsageState();
            if (OldState == NewState) return;

            Barrier.Transition.StateBefore = OldState;
            Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            mResourceBarriers.push_back(Barrier);
        }
        else
        {
            const uint32_t NumSub = pResource->GetNumSubresources();
            for (uint32_t Sub = 0; Sub < NumSub; Sub++)
            {
                const D3D12_RESOURCE_STATES OldState = pResource->GetSubresourceState(Sub);
                if (OldState == NewState) continue;

                Barrier.Transition.StateBefore = OldState;
                Barrier.Transition.Subresource = Sub;
                mResourceBarriers.push_back(Barrier);
            }
        }
        pResource->SetUsageState(NewState);
    }
    else
    {
        const D3D12_RESOURCE_STATES OldState = pResource->GetSubresourceState(Subresource);
        if (OldState != NewState)
        {
            Barrier.Transition.StateBefore = OldState;
            Barrier.Transition.Subresource = Subresource;
            mResourceBarriers.push_back(Barrier);
        }
        pResource->SetSubresourceState(Subresource, NewState);
    }

    if (!bFlushImmediate || mResourceBarriers.size() >= MaxBatchedBarriers)
    {
        FlushResourceBarriers();
    }
}

void FD3D12CommandContext::InsertUAVBarrier(D3D12GpuResource *pResource, bool bFlushImmediate)
{
    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    Barrier.UAV.pResource = pResource->GetResource();

    mResourceBarriers.push_back(Barrier);

    if (bFlushImmediate || mResourceBarriers.size() >= MaxBatchedBarriers)
    {
        FlushResourceBarriers();
    }
}

void FD3D12CommandContext::InsertAliasingBarrier(D3D12GpuResource *pBefore, D3D12GpuResource *pAfter, bool bFlushImmediate)
{
    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    Barrier.Aliasing.pResourceBefore = pBefore ? pBefore->GetResource() : nullptr;
    Barrier.Aliasing.pResourceAfter = pAfter ? pAfter->GetResource() : nullptr;

    mResourceBarriers.push_back(Barrier);

    if (bFlushImmediate || mResourceBarriers.size() >= MaxBatchedBarriers)
    {
        FlushResourceBarriers();
    }
}

void FD3D12CommandContext::FlushResourceBarriers()
{
    if (mResourceBarriers.empty())
    {
        return;
    }

    mpCommandList->ResourceBarrier(static_cast<UINT>(mResourceBarriers.size()), mResourceBarriers.data());
    mResourceBarriers.clear();
}

void FD3D12CommandContext::SetGraphicsRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const
{
    mpCommandList->SetGraphicsRootDescriptorTable(RootIndex, BaseDescriptor);
}

void FD3D12CommandContext::SetComputeRootDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) const
{
    mpCommandList->SetComputeRootDescriptorTable(RootIndex, BaseDescriptor);
}

void FD3D12CommandContext::SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature) const
{
    mpCommandList->SetGraphicsRootSignature(pRootSignature);
}

void FD3D12CommandContext::SetComputeRootSignature(ID3D12RootSignature* pRootSignature) const
{
    mpCommandList->SetComputeRootSignature(pRootSignature);
}

void FD3D12CommandContext::SetPipelineState(ID3D12PipelineState* pPipelineState) const
{
    mpCommandList->SetPipelineState(pPipelineState);
}

void FD3D12CommandContext::SetViewport(const D3D12_VIEWPORT& Viewport) const
{
    mpCommandList->RSSetViewports(1, &Viewport);
}

void FD3D12CommandContext::SetScissorRect(const D3D12_RECT& Rect) const
{
    mpCommandList->RSSetScissorRects(1, &Rect);
}

void FD3D12CommandContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVs,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDSV) const
{
    mpCommandList->OMSetRenderTargets(NumRTVs, pRTVs, FALSE, pDSV);
}

void FD3D12CommandContext::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float Color[4])
{
    FlushResourceBarriers();
    mpCommandList->ClearRenderTargetView(RTV, Color, 0, nullptr);
}

void FD3D12CommandContext::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DSV, D3D12_CLEAR_FLAGS ClearFlags, float Depth,
    UINT8 Stencil)
{
    FlushResourceBarriers();
    mpCommandList->ClearDepthStencilView(DSV, ClearFlags, Depth, Stencil, 0, nullptr);
}

void FD3D12CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology) const
{
    mpCommandList->IASetPrimitiveTopology(Topology);
}

void FD3D12CommandContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation,
    UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    mpCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void FD3D12CommandContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
    INT BaseVertexLocation, UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    mpCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void FD3D12CommandContext::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    FlushResourceBarriers();
    mpCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void FD3D12CommandContext::SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) const
{
    mpCommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
}

void FD3D12CommandContext::SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet,
    const void* pSrcData, UINT DestOffsetIn32BitValues)
{
    mpCommandList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void FD3D12CommandContext::SetGraphicsRootConstantBufferView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    mpCommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
}

void FD3D12CommandContext::SetGraphicsRootShaderResourceView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetGraphicsRootShaderResourceView(RootParameterIndex, BufferLocation);
}

void FD3D12CommandContext::SetGraphicsRootUnorderedAccessView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetGraphicsRootUnorderedAccessView(RootParameterIndex, BufferLocation);
}

void FD3D12CommandContext::SetComputeRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet,
    const void *pSrcData, UINT DestOffsetIn32BitValues) const
{
    mpCommandList->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void FD3D12CommandContext::SetComputeRootConstantBufferView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
}

void FD3D12CommandContext::SetComputeRootShaderResourceView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetComputeRootShaderResourceView(RootParameterIndex, BufferLocation);
}

void FD3D12CommandContext::SetComputeRootUnorderedAccessView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetComputeRootUnorderedAccessView(RootParameterIndex, BufferLocation);
}

void FD3D12CommandContext::CopyBufferRegion(ID3D12Resource* pDstBuffer, UINT64 DstOffset, ID3D12Resource* pSrcBuffer,
                                       UINT64 SrcOffset, UINT64 NumBytes)
{
    // A buffer copy relies on the source being in a read state and dest in a write state.
    // We must flush barriers before executing the copy to ensure state changes take effect.
    FlushResourceBarriers();
    mpCommandList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
}

namespace
{
    constexpr UINT kPixEventUnicodeMetadata = 1;
}

void FD3D12CommandContext::BeginEvent(const char *Name) const
{
    if (!Name) return;
    const std::wstring Wide = StringUtils::UTF8ToWide(Name);
    mpCommandList->BeginEvent(kPixEventUnicodeMetadata, Wide.c_str(), static_cast<UINT>((Wide.size() + 1) * sizeof(wchar_t)));
}

void FD3D12CommandContext::EndEvent() const
{
    mpCommandList->EndEvent();
}

void FD3D12CommandContext::SetMarker(const char *Name) const
{
    if (!Name) return;
    const std::wstring Wide = StringUtils::UTF8ToWide(Name);
    mpCommandList->SetMarker(kPixEventUnicodeMetadata, Wide.c_str(), static_cast<UINT>((Wide.size() + 1) * sizeof(wchar_t)));
}

void FD3D12CommandContext::IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) const
{
    mpCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

void FD3D12CommandContext::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pViews) const
{
    mpCommandList->IASetIndexBuffer(pViews);
}
