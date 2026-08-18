#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Resource/GpuResource.h"
#include "Renderer/D3D12Core/Common.h"

#include <cassert>

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

void FCommandContext::Begin()
{
    mpCommandAllocator->Reset();
    mpCommandList->Reset(mpCommandAllocator.Get(), nullptr);

    mResourceBarriers.clear();
}

void FCommandContext::Close()
{
    FlushResourceBarriers();
    mpCommandList->Close();
}

void FCommandContext::TransitionResource(GpuResource *pResource, D3D12_RESOURCE_STATES NewState, uint32_t Subresource,
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

void FCommandContext::InsertUAVBarrier(GpuResource *pResource, bool bFlushImmediate)
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

void FCommandContext::InsertAliasingBarrier(GpuResource *pBefore, GpuResource *pAfter, bool bFlushImmediate)
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

void FCommandContext::SetGraphicsRootShaderResourceView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetGraphicsRootShaderResourceView(RootParameterIndex, BufferLocation);
}

void FCommandContext::SetGraphicsRootUnorderedAccessView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetGraphicsRootUnorderedAccessView(RootParameterIndex, BufferLocation);
}

void FCommandContext::SetComputeRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet,
    const void *pSrcData, UINT DestOffsetIn32BitValues) const
{
    mpCommandList->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void FCommandContext::SetComputeRootConstantBufferView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
}

void FCommandContext::SetComputeRootShaderResourceView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetComputeRootShaderResourceView(RootParameterIndex, BufferLocation);
}

void FCommandContext::SetComputeRootUnorderedAccessView(UINT RootParameterIndex,
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) const
{
    mpCommandList->SetComputeRootUnorderedAccessView(RootParameterIndex, BufferLocation);
}

void FCommandContext::CopyBufferRegion(ID3D12Resource* pDstBuffer, UINT64 DstOffset, ID3D12Resource* pSrcBuffer,
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

void FCommandContext::BeginEvent(const char *Name) const
{
    if (!Name) return;
    const std::wstring Wide = StringUtils::UTF8ToWide(Name);
    mpCommandList->BeginEvent(kPixEventUnicodeMetadata, Wide.c_str(), static_cast<UINT>((Wide.size() + 1) * sizeof(wchar_t)));
}

void FCommandContext::EndEvent() const
{
    mpCommandList->EndEvent();
}

void FCommandContext::SetMarker(const char *Name) const
{
    if (!Name) return;
    const std::wstring Wide = StringUtils::UTF8ToWide(Name);
    mpCommandList->SetMarker(kPixEventUnicodeMetadata, Wide.c_str(), static_cast<UINT>((Wide.size() + 1) * sizeof(wchar_t)));
}

void FCommandContext::IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) const
{
    mpCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

void FCommandContext::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pViews) const
{
    mpCommandList->IASetIndexBuffer(pViews);
}
