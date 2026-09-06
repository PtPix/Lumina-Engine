#include "Passes/RenderPass.h"
#include "D3D12CommandContext.h"

void FGraphicsPassBase::BindRenderTargets(FD3D12CommandContext *CommandContext, const D3D12_CPU_DESCRIPTOR_HANDLE *RTVs,
                                          uint32_t NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE *DSV, uint32_t ViewportWidth, uint32_t ViewportHeight)
{
    if (!CommandContext) return;

    CommandContext->SetRenderTargets(NumRTVs, RTVs, DSV);

    D3D12_VIEWPORT Viewport = {};
    Viewport.TopLeftX = 0.0f;
    Viewport.TopLeftY = 0.0f;
    Viewport.Width = static_cast<float>(ViewportWidth);
    Viewport.Height = static_cast<float>(ViewportHeight);
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    CommandContext->SetViewport(Viewport);

    D3D12_RECT ScissorRect = {};
    ScissorRect.left = 0;
    ScissorRect.top = 0;
    ScissorRect.right = static_cast<LONG>(ViewportWidth);
    ScissorRect.bottom = static_cast<LONG>(ViewportHeight);
    CommandContext->SetScissorRect(ScissorRect);
}

void FComputePassBase::Dispatch(FD3D12CommandContext *CommandContext, uint32_t ThreadGroupX, uint32_t ThreadGroupY,
    uint32_t ThreadGroupZ)
{
    if (!CommandContext) return;

    CommandContext->Dispatch(ThreadGroupX, ThreadGroupY, ThreadGroupZ);
}
