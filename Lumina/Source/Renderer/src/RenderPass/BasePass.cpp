#include "Renderer/RenderPass/BasePass.h"

#include "Renderer/Renderer.h"
#include "Renderer/Pipeline/PipelineStateCache.h"
#include "Renderer/Scene/FSceneView.h"
#include "Renderer/Resources/FMesh.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/Pipeline/GlobalRootSignature.h"

namespace
{
    FGraphicsPSODesc MakeBasePassPSODesc()
    {
        FGraphicsPSODesc Desc;
        Desc.ShaderPath   = L"Shaders/BasePass.hlsl";
        Desc.VSEntry      = "VSMain";
        Desc.PSEntry      = "PSMain";
        Desc.ShaderModel  = EShaderModel::SM6_0;
        Desc.InputLayout  = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        Desc.RTVFormats    = { DXGI_FORMAT_R8G8B8A8_UNORM };

        Desc.DSVFormat     = DXGI_FORMAT_D32_FLOAT;
        Desc.DepthTest     = EDepthTest::Greater;
        Desc.bDepthWrite   = true;

        Desc.CullMode      = ECullMode::Back;
        Desc.BlendMode     = EBlendMode::Opaque;
        Desc.TopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        Desc.RootSignature = FGlobalRootSignature::GetGraphicsRootSignature();

        return Desc;
    }
}

void AddBasePass(FRenderGraph& Graph,
                 const FBasePassInputs& Inputs,
                 FBasePassOutputs& Outputs)
{
    const uint32_t Width  = Inputs.ViewportWidth  ? Inputs.ViewportWidth
                                                  : Renderer::GetD3D12Backend()->GetWidth();
    const uint32_t Height = Inputs.ViewportHeight ? Inputs.ViewportHeight
                                                  : Renderer::GetD3D12Backend()->GetHeight();

    // --- 创建本 pass 拥有的深度目标 ---
    FRGTextureDesc DepthDesc = {};
    DepthDesc.Width  = Width;
    DepthDesc.Height = Height;
    DepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.Usage  = ERGTextureUsage::DepthStencil;
    DepthDesc.bUseClearValue = true;
    DepthDesc.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.ClearValue.DepthStencil.Depth = 0.0f; // Reverse-Z
    DepthDesc.ClearValue.DepthStencil.Stencil = 0;

    Outputs.SceneDepth = Graph.CreateTexture("SceneDepth", DepthDesc);

    const FSceneView* View = Inputs.View;   // 捕获指针(View 生命周期 >= 帧)

    // --- 声明 pass(粒度保持显式)---
    Graph.AddPass("BasePass")
        .WriteRenderTarget(Inputs.SceneColor, ERGLoadOp::Clear)
        .WriteDepth(Outputs.SceneDepth, ERGLoadOp::Clear)
        .Execute([View, Width, Height](FRenderGraphContext& Context)
        {
            FCommandContext* Cmd = Context.GetCommandContext();

            // 2. viewport / scissor
            D3D12_VIEWPORT Viewport = { 0.0f, 0.0f,
                static_cast<float>(Width), static_cast<float>(Height), 0.0f, 1.0f };
            D3D12_RECT Scissor = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
            Cmd->SetViewport(Viewport);
            Cmd->SetScissorRect(Scissor);

            // 4. PSO(走缓存)
            FPipelineState* PSO = FPipelineStateCache::GetOrCreate(MakeBasePassPSODesc());
            if (!PSO) return;
            Cmd->SetPipelineState(PSO->Get());
            Cmd->SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // // 5. 绘制
            // for (const auto& DrawCmd : View->DrawCommands)
            // {
            //     Cmd->SetGraphicsRoot32BitConstants(
            //         ToRootIndex(ERootParam::PerObjectConstant), 1, &DrawCmd.InstanceIndex, 0);
            //     DrawCmd.pMesh->Draw(Cmd);
            // }
        });
}