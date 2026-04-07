#include <cassert>

#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"

CommandContext::~CommandContext()
{
    mpCommandList.Reset();
    mpCommandAllocator.Reset();
}

bool CommandContext::Initialize(Device* pDevice, ECommandQueueType Type, ID3D12CommandAllocator* pAllocator)
{
    assert(pDevice != nullptr);
    assert(pAllocator != nullptr);

    mpDevice = pDevice;
    mpCommandAllocator = pAllocator;
    mType = Type;

    D3D12_COMMAND_LIST_TYPE D3D12Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    switch (Type)
    {
    case COMPUTE: D3D12Type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
    case COPY: D3D12Type = D3D12_COMMAND_LIST_TYPE_COPY; break;
    default: break;
    }

    HRESULT HResult = mpDevice->GetDevicePtr()->CreateCommandList(
        0, D3D12Type, mpCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList)
        );
    if (FAILED(HResult))
    {
        LUMINA_LOG_ERROR(RHI, "CommandContext::Initialize failed to create Command List.");
        return false;
    }

    mpCommandList->Close();
    mResourceBarriers.reserve(16);

    return true;
}

void CommandContext::Begin()
{
    mpCommandAllocator.Get()->Reset();
    mpCommandList->Reset(mpCommandAllocator.Get(), nullptr);

    mResourceBarriers.clear();
}

void CommandContext::Close()
{
    FlushResourceBarriers();
    mpCommandList->Close();
}

void CommandContext::TransitionResource(GpuResource* pResource, D3D12_RESOURCE_STATES NewState, bool bFlushImmediate)
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

void CommandContext::FlushResourceBarriers()
{
    if (mResourceBarriers.empty())
    {
        return;
    }

    mpCommandList->ResourceBarrier(static_cast<UINT>(mResourceBarriers.size()), mResourceBarriers.data());
    mResourceBarriers.clear();
}

void CommandContext::SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature)
{
    mpCommandList->SetGraphicsRootSignature(pRootSignature);
}

void CommandContext::SetComputeRootSignature(ID3D12RootSignature* pRootSignature)
{
    mpCommandList->SetComputeRootSignature(pRootSignature);
}

void CommandContext::SetPipelineState(ID3D12PipelineState* pPipelineState)
{
    mpCommandList->SetPipelineState(pPipelineState);
}

void CommandContext::SetViewport(const D3D12_VIEWPORT& Viewport)
{
    mpCommandList->RSSetViewports(1, &Viewport);
}

void CommandContext::SetScissorRect(const D3D12_RECT& Rect)
{
    mpCommandList->RSSetScissorRects(1, &Rect);
}

void CommandContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVs,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDSV)
{
    mpCommandList->OMSetRenderTargets(NumRTVs, pRTVs, FALSE, pDSV);
}

void CommandContext::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float Color[4])
{
    FlushResourceBarriers();
    mpCommandList->ClearRenderTargetView(RTV, Color, 0, nullptr);
}

void CommandContext::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DSV, D3D12_CLEAR_FLAGS ClearFlags, float Depth,
    UINT8 Stencil)
{
    FlushResourceBarriers();
    mpCommandList->ClearDepthStencilView(DSV, ClearFlags, Depth, Stencil, 0, nullptr);
}

void CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
{
    mpCommandList->IASetPrimitiveTopology(Topology);
}

void CommandContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation,
    UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    mpCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void CommandContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
    INT BaseVertexLocation, UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    mpCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void CommandContext::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    mpCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void CommandContext::SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps)
{
    mpCommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
}
