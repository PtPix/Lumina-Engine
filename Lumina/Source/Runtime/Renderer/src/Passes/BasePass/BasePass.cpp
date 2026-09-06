#include "Passes/BasePass/BasePass.h"
#include "Scene/ViewInfo.h"
#include "Scene/GPUScene.h"
#include "RenderCore.h"
#include "D3D12CommandContext.h"

FBasePass::FBasePass(const FBasePassInputs &Inputs) : FGraphicsPassBase("BasePass"), mInputs(Inputs) {}

void FBasePass::Setup(FRenderGraphPassBuilder &Builder)
{
    Builder.WriteRenderTarget(mInputs.SceneColor, ERGLoadOp::Clear);

    FRGTextureDesc DepthDesc = {};
    DepthDesc.Width = FRendererCore::GetRenderWidth();
    DepthDesc.Height = FRendererCore::GetRenderHeight();
    DepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.Usage = ERGTextureUsage::DepthStencil;
    DepthDesc.bUseClearValue = true;
    DepthDesc.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.ClearValue.DepthStencil.Depth = 0.0f;
    DepthDesc.ClearValue.DepthStencil.Stencil = 0;

    FRenderGraph* RenderGraph = Builder.GetRenderGraph();
    mOutputs.SceneDepth = RenderGraph->CreateTexture("SceneDepth", DepthDesc);

    Builder.WriteDepth(mOutputs.SceneDepth, ERGLoadOp::Clear);

    // Setup execution callback
    Builder.Execute([this](FRenderGraphContext& RGContext)
    {
        FPassContext Context = {};
        Context.RGContext = &RGContext;
        Context.ViewInfo = nullptr;  // TODO: Get from SceneRenderer
        Context.GPUScene = FRendererCore::GetGPUScene();

        Execute(Context);
    });
}

void FBasePass::Execute(const FPassContext &Context)
{
    auto* CommandContext = Context.GetCommandContext();
    if (!CommandContext) return;

    D3D12_CPU_DESCRIPTOR_HANDLE RTV = Context.RGContext->GetRTV(mInputs.SceneColor);
    D3D12_CPU_DESCRIPTOR_HANDLE DSV = Context.RGContext->GetDSV(mOutputs.SceneDepth);

    BindRenderTargets(CommandContext, &RTV, 1, &DSV,
        FRendererCore::GetRenderWidth(), FRendererCore::GetRenderHeight());

    // TODO: Get draw commands from SceneRenderer
    // For now, just clear
    float ClearColor[4] = { 0.2f, 0.3f, 0.4f, 1.0f };
    CommandContext->ClearRenderTargetView(RTV, ClearColor);
    CommandContext->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0);
}
