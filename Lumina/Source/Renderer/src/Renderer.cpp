#include "Renderer/Renderer.h"

#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"

bool Renderer::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    return D3D12Backend::Initialize(Hwnd, Width, Height);;
}

void Renderer::Shutdown()
{
    D3D12Backend::Shutdown();
}

FCommandContext* Renderer::BeginFrame()
{
    D3D12Backend::BeginFrame();

    FCommandContext* pContext = D3D12Backend::AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    pContext->FlushResourceBarriers();

    return pContext;
}

void Renderer::EndFrame(FCommandContext* pContext)
{
    if (!pContext) return;

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT);
    pContext->Close();

    D3D12Backend::GetGraphicsQueue()->ExecuteCommandList(pContext->GetCommandList());

    D3D12Backend::FreeContext(pContext);

    D3D12Backend::EndFrameAndPresent();
    D3D12Backend::FlushGPU();
}