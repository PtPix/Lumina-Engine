#include "Passes/DebugPasses/DebugVisualizePass.h"
#include "RenderCore.h"
#include "D3D12CommandContext.h"
#include "Shaders/GlobalRootSignature.h"

FDebugVisualizePass::FDebugVisualizePass(const FDebugVisualizeInputs &Inputs)
    : FFullscreenPassBase("DebugVisualizePass"), mInputs(Inputs) {}

void FDebugVisualizePass::Setup(FRenderGraphPassBuilder &Builder)
{
    Builder.ReadTexture(mInputs.InputTexture);

    Builder.WriteRenderTarget(mInputs.OutputTexture, ERGLoadOp::Load);

    Builder.Execute([this](FRenderGraphContext& RGContext)
    {
        FPassContext Context = {};
        Context.RGContext = &RGContext;
        Context.ViewInfo = nullptr;
        Context.GPUScene = FRendererCore::GetGPUScene();

        Execute(Context);
    });
}

void FDebugVisualizePass::Execute(const FPassContext &Context)
{
    auto* CommandContext = Context.GetCommandContext();
    if (!CommandContext) return;

    D3D12_CPU_DESCRIPTOR_HANDLE RTV = Context.RGContext->GetRTV(mInputs.OutputTexture);

    BindRenderTargets(CommandContext, &RTV, 1, nullptr,
        FRendererCore::GetRenderWidth(), FRendererCore::GetRenderHeight());

    CommandContext->SetGraphicsRootSignature(FGlobalRootSignature::GetGraphicsRootSignature());

    float DebugColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    CommandContext->ClearRenderTargetView(RTV, DebugColor);
}
