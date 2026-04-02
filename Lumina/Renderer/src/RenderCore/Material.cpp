#include "Renderer/RenderCore/Material.h"
#include "Renderer/RenderCore/RootSignature.h"
#include "Renderer/RenderCore/ShaderCompiler.h"
#include "Logger/Logger.h"
#include <dxcapi.h>
bool MaterialBase::InitializePipeline(ID3D12Device* Device, const FMaterialInitDesc& MaterialDesc)
{
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

    GraphicsPipelineStateBuilder Builder;
    Builder.SetRootSignature(mRootSignature->Get())
        .SetInputLayout(MaterialDesc.InputElements)
        .SetRenderTargetFormats(MaterialDesc.RenderTargetViewFormats)
        .SetDepthStencilFormat(MaterialDesc.DepthStencilViewFormat);

    if (MaterialDesc.bEnableDepthTest)
    {
        Builder.EnableDepthTest();
    }
    if (!VertexShaderBlob.IsNull())
    {
        Builder.SetVertexShader(VertexShaderBlob.GetByteCode(), VertexShaderBlob.GetByteCodeSize());
    }
    if (!PixelShaderBlob.IsNull())
    {
        Builder.SetPixelShader(PixelShaderBlob.GetByteCode(), PixelShaderBlob.GetByteCodeSize());
    }

    Builder.Build(Device, mPipelineState);

    if (!mPipelineState.Get())
    {
        Log::Error("Failed to build Pipeline State!");
        return false;
    }

    return true;
}
