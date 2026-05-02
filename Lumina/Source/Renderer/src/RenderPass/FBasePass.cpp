#include "Renderer/RenderPass/FBasePass.h"

#include "Renderer/Renderer.h"
#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/D3D12Core/Pipeline/ShaderCompiler.h"

void FBasePass::Initialize()
{
    std::string ErrorString;
    ShaderUtils::FBlob VertexShaderBlob;
    ShaderUtils::FBlob PixelShaderBlob;

    FShaderStageCompileDesc VertexShaderStageDesc = {
        L"Shaders/BasePass.hlsl", "VSMain", EShaderStage::VertexShader, EShaderModel::SM6_0
    };
    VertexShaderBlob = ShaderUtils::CompileFromSource(VertexShaderStageDesc, ErrorString);
    if (VertexShaderBlob.IsNull())
    {
        Log::Error("BasePass VS Compile Failed: %s", ErrorString.c_str());
        return;
    }

    FShaderStageCompileDesc PixelShaderStageDesc = {
        L"Shaders/BasePass.hlsl", "PSMain", EShaderStage::PixelShader, EShaderModel::SM6_0
    };
    PixelShaderBlob = ShaderUtils::CompileFromSource(PixelShaderStageDesc, ErrorString);
    if (PixelShaderBlob.IsNull())
    {
        Log::Error("BasePass PS Compile Failed: %s", ErrorString.c_str());
        return;
    }

    // 3. 配置输入布局 (与 FStandardVertex 对应)
    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 4. 构建 Graphics Pipeline State
    GraphicsPipelineStateBuilder Builder;
    Builder.SetRootSignature(Renderer::GetBindlessRootSignature()->Get())
           .SetInputLayout(InputElements)
           // BasePass 专注于 RT 的输出，这里填入你 G-Buffer 或后备缓冲的格式
           .SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM })
           .SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT);

    Builder.EnableDepthTest(); // 开启深度测试以保证正确的遮挡关系

    if (!VertexShaderBlob.IsNull())
    {
        Builder.SetVertexShader(VertexShaderBlob.GetByteCode(), VertexShaderBlob.GetByteCodeSize());
    }
    if (!PixelShaderBlob.IsNull())
    {
        Builder.SetPixelShader(PixelShaderBlob.GetByteCode(), PixelShaderBlob.GetByteCodeSize());
    }

    mBasePassPSO = std::make_unique<PipelineState>();
    Builder.Build(D3D12Backend::GetDevice()->GetDevice(), *mBasePassPSO);

}

void FBasePass::Execute(FCommandContext* pCommandContext, const FSceneView& View)
{
    pCommandContext->SetPipelineState(mBasePassPSO->Get());
    pCommandContext->SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const auto& Cmd : View.DrawCommands)
    {
        pCommandContext->SetGraphicsRoot32BitConstants(0, 1, &Cmd.InstanceIndex, 0);
        Cmd.pMesh->Draw(pCommandContext);
    }
}

void FBasePass::Shutdown()
{
    mBasePassPSO->Destroy();
}
