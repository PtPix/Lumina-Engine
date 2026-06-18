#include "Renderer/RenderPass/FBasePass.h"

#include "Renderer/Renderer.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/RendererCommon.h"
#include "Renderer/Pipeline/PipelineStateCache.h"

namespace
{
    FGraphicsPSODesc MakeBasePassPSODesc()
    {
        FGraphicsPSODesc Desc;
        Desc.ShaderPath  = L"Shaders/BasePass.hlsl";
        Desc.VSEntry     = "VSMain";
        Desc.PSEntry     = "PSMain";
        Desc.ShaderModel = EShaderModel::SM6_0;
        Desc.InputLayout = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        Desc.RTVFormats   = { DXGI_FORMAT_R8G8B8A8_UNORM };
        Desc.DSVFormat    = DXGI_FORMAT_D32_FLOAT;
        Desc.bDepthTest   = true;
        Desc.RootSignature = Renderer::GetBindlessRootSignature()->Get();
        return Desc;
    }
}

void FBasePass::Initialize(FDevice* pDevice)
{
    FPipelineStateCache::GetOrCreate(MakeBasePassPSODesc());
}

void FBasePass::Execute(FCommandContext* pCommandContext, const FSceneView& View)
{
    FPipelineState* PSO = FPipelineStateCache::GetOrCreate(MakeBasePassPSODesc());
    if (!PSO) return;

    pCommandContext->SetPipelineState(PSO->Get());
    pCommandContext->SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const auto& Cmd : View.DrawCommands)
    {
        pCommandContext->SetGraphicsRoot32BitConstants(ToRootIndex(ERootParam::PerObjectConstant), 1, &Cmd.InstanceIndex, 0);
        Cmd.pMesh->Draw(pCommandContext);
    }
}

void FBasePass::Shutdown()
{
}
