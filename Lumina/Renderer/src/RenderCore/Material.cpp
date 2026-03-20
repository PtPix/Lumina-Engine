#include "../../include/Renderer/RenderCore/Material.h"

#include "Logger/Logger.h"
#include "../../include/Renderer/RenderCore/RootSignature.h"
#include "../../include/Renderer/RenderCore/ShaderCompiler.h"

#include <dxcapi.h>

bool Material::Initialize(ID3D12Device* Device, const FMaterialInitDesc& MaterialDesc)
{
    // Compile Shaders
    std::string ErrorString;
    ShaderUtils::FBlob VertexShaderBlob;
    ShaderUtils::FBlob PixelShaderBlob;

    if (!MaterialDesc.VertexShaderFilePath.empty())
    {
        FShaderStageCompileDesc VertexShaderStageDesc =
        {
            MaterialDesc.VertexShaderFilePath, MaterialDesc.VertexShaderEntryPoint, EShaderStage::VertexShader, EShaderModel::SM6_0
        };
        VertexShaderBlob = ShaderUtils::CompileFromSource(VertexShaderStageDesc, ErrorString);
        if (VertexShaderBlob.IsNull())
        {
            Log::Error("VS Compile Failed: %s", ErrorString.c_str());
            return false;
        }
    }

    if (!MaterialDesc.PixelShaderFilePath.empty())
    {
        FShaderStageCompileDesc PixelShaderStageDesc =
        {
            MaterialDesc.PixelShaderFilePath, MaterialDesc.PixelShaderEntryPoint, EShaderStage::PixelShader, EShaderModel::SM6_0
        };
        PixelShaderBlob = ShaderUtils::CompileFromSource(PixelShaderStageDesc, ErrorString);
        if (PixelShaderBlob.IsNull())
        {
            Log::Error("PS Compile Failed: %s", ErrorString.c_str());
            return false;
        }
    }

    GraphicsPipelineStateBuilder GraphicsPipelineStateBuilder;
    GraphicsPipelineStateBuilder.SetRootSignature(mRootSignature->Get())
        .SetInputLayout(MaterialDesc.InputElements)
        .SetRenderTargetFormats(MaterialDesc.RenderTargetViewFormats)
        .SetDepthStencilFormat(MaterialDesc.DepthStencilViewFormat);

    if (MaterialDesc.bEnableDepthTest)
    {
        GraphicsPipelineStateBuilder.EnableDepthTest();
    }
    if (!VertexShaderBlob.IsNull())
    {
        GraphicsPipelineStateBuilder.SetVertexShader(VertexShaderBlob.GetByteCode(), VertexShaderBlob.GetByteCodeSize());
    }
    if (!PixelShaderBlob.IsNull())
    {
        GraphicsPipelineStateBuilder.SetPixelShader(PixelShaderBlob.GetByteCode(), PixelShaderBlob.GetByteCodeSize());
    }

    GraphicsPipelineStateBuilder.Build(Device, mPipelineState);

    return true;
}

void Material::Bind(ID3D12GraphicsCommandList* CommandList) const
{
    if (mRootSignature->Get() && mPipelineState.Get())
    {
        CommandList->SetPipelineState(mPipelineState.Get());
    }
}

void Material::Destroy()
{
    if (mPipelineState.Get())
    {
        mPipelineState.Destroy();
    }
    if (mRootSignature->Get())
    {
        mRootSignature->Destroy();
    }
}
