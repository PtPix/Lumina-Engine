#include "Renderer/RenderCore/Material.h"
#include "../../include/Renderer/D3D12Core/RootSignature.h"
#include "../../include/Renderer/D3D12Core/ShaderCompiler.h"
#include "Renderer/D3D12Core/Texture.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Resource/Texture.h"

void MaterialBase::SetTexture(UINT RootIndex, UINT Offset, Texture* pTexture)
{
    if (!pTexture) return;

    for (auto& Binding : mBoundTextures)
    {
        if (Binding.RootIndex == RootIndex && Binding.Offset == Offset)
        {
            Binding.pTexture = pTexture;
            return;
        }
    }

    mBoundTextures.push_back({ RootIndex, Offset, pTexture });
}

void MaterialBase::SetTextures(UINT RootIndex, UINT StartOffset, Texture** ppTexture, UINT NumTextures)
{
    for (UINT i = 0; i < NumTextures; i++)
    {
        SetTexture(RootIndex, StartOffset + i, ppTexture[i]);
    }
}

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
        LUMINA_LOG_WARNING(Material, "Failed to build Pipeline State!");
        return false;
    }

    return true;
}

void MaterialBase::BindTextures(FCommandContext& Context) const
{
    for (const auto& Binding : mBoundTextures)
    {
        if (Binding.pTexture)
        {
            Context.SetGraphicsRootDescriptorTable(Binding.RootIndex, Binding.Offset, Binding.pTexture->GetSRV());
        }
    }
}
