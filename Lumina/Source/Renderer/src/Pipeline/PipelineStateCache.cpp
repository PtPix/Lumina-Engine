#include "Renderer/Pipeline/PipelineStateCache.h"

#include "Renderer/Pipeline/ShaderManager.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Pipeline/ShaderCompiler.h"

#include <functional>

FDevice* FPipelineStateCache::mpDevice = nullptr;
std::unordered_map<size_t, std::unique_ptr<FPipelineState>> FPipelineStateCache::mCache;

void FPipelineStateCache::Initialize(FDevice* pDevice)
{
    mpDevice = pDevice;
}

void FPipelineStateCache::Shutdown()
{
    mCache.clear();
    mpDevice = nullptr;
}

static void HashCombine(size_t& Seed, size_t Value)
{
    Seed ^= Value + 0x9e3779b97f4a7c15ull + (Seed << 6) + (Seed >> 2);
}

size_t FPipelineStateCache::MakeHash(const FGraphicsPSODesc& Desc)
{
    size_t Seed = 0;
    HashCombine(Seed, std::hash<std::wstring>{}(Desc.ShaderPath));
    HashCombine(Seed, std::hash<std::string>{}(Desc.VSEntry));
    HashCombine(Seed, std::hash<std::string>{}(Desc.PSEntry));
    HashCombine(Seed, static_cast<size_t>(Desc.ShaderModel));
    HashCombine(Seed, static_cast<size_t>(Desc.DSVFormat));
    HashCombine(Seed, static_cast<size_t>(Desc.bDepthTest));
    HashCombine(Seed, static_cast<size_t>(Desc.TopologyType));
    for (DXGI_FORMAT Fmt : Desc.RTVFormats)
        HashCombine(Seed, static_cast<size_t>(Fmt));
    for (const D3D12_INPUT_ELEMENT_DESC& E : Desc.InputLayout)
    {
        HashCombine(Seed, std::hash<const char*>{}(E.SemanticName));
        HashCombine(Seed, static_cast<size_t>(E.Format));
        HashCombine(Seed, static_cast<size_t>(E.AlignedByteOffset));
    }
    HashCombine(Seed, reinterpret_cast<size_t>(Desc.RootSignature));
    return Seed;
}

FPipelineState* FPipelineStateCache::GetOrCreate(const FGraphicsPSODesc& Desc)
{
    const size_t Hash = MakeHash(Desc);
    if (auto It = mCache.find(Hash); It != mCache.end())
    {
        return It->second.get();
    }

    // --- Fetch shaders (cached) ---
    FShaderStageCompileDesc VSDesc;
    VSDesc.FilePath    = Desc.ShaderPath;
    VSDesc.EntryPoint  = Desc.VSEntry;
    VSDesc.ShaderStage = EShaderStage::VertexShader;
    VSDesc.ShaderModel = Desc.ShaderModel;

    FShaderStageCompileDesc PSDesc;
    PSDesc.FilePath    = Desc.ShaderPath;
    PSDesc.EntryPoint  = Desc.PSEntry;
    PSDesc.ShaderStage = EShaderStage::PixelShader;
    PSDesc.ShaderModel = Desc.ShaderModel;

    ShaderUtils::FBlob* VS = FShaderManager::GetShader(VSDesc);
    ShaderUtils::FBlob* PS = FShaderManager::GetShader(PSDesc);
    if (!VS || !PS)
    {
        LUMINA_LOG_ERROR(RHI, "FPipelineStateCache: shader fetch failed for '%s'",
                         StringUtils::WideToUTF8(Desc.ShaderPath).c_str());
        return nullptr;
    }

    // --- Build PSO ---
    FGraphicsPipelineStateBuilder Builder;
    Builder.SetRootSignature(Desc.RootSignature)
           .SetInputLayout(Desc.InputLayout)
           .SetRenderTargetFormats(Desc.RTVFormats, Desc.DSVFormat)
           .SetPrimitiveTopologyType(Desc.TopologyType)
           .SetVertexShader(VS->GetByteCode(), VS->GetByteCodeSize())
           .SetPixelShader(PS->GetByteCode(), PS->GetByteCodeSize());

    if (Desc.bDepthTest)
    {
        Builder.EnableDepthTest();
    }
    if (Desc.DSVFormat != DXGI_FORMAT_UNKNOWN)
    {
        Builder.SetDepthStencilFormat(Desc.DSVFormat);
    }

    auto PSO = std::make_unique<FPipelineState>();
    if (!Builder.Build(mpDevice->GetDevice(), *PSO))
    {
        return nullptr;
    }

    FPipelineState* Result = PSO.get();
    mCache.emplace(Hash, std::move(PSO));

    return Result;
}