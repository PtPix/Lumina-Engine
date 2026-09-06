#include "Passes/FullscreenPass.h"
#include "D3D12CommandContext.h"

void FFullscreenPassBase::DrawFullscreenTriangle(FD3D12CommandContext *CommandContext)
{
    if (!CommandContext) return;

    CommandContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    CommandContext->DrawInstanced(3, 1, 0, 0);
}
