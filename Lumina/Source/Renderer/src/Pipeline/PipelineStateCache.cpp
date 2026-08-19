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

void FPipelineStateCache::InvalidateAll()
{
    mCache.clear();
}

size_t FPipelineStateCache::GetCachedCount()
{
    return mCache.size();
}

// ---------------------------------------------------------------------------
// Hash
// ---------------------------------------------------------------------------

static void HashCombine(size_t& Seed, size_t Value)
{
    Seed ^= Value + 0x9e3779b97f4a7c15ull + (Seed << 6) + (Seed >> 2);
}

static void HashDefines(size_t& Seed, const std::vector<FShaderDefine>& Defines)
{
    for (const FShaderDefine& Define : Defines)
    {
        HashCombine(Seed, std::hash<std::string>{}(Define.Name));
        HashCombine(Seed, std::hash<std::string>{}(Define.Value));
    }
}

size_t FPipelineStateCache::MakeHash(const FGraphicsPSODesc& Desc)
{
    size_t Seed = 0x9E3779B9;

    HashCombine(Seed, std::hash<std::wstring>{}(Desc.ShaderPath));
    HashCombine(Seed, std::hash<std::string>{}(Desc.VSEntry));
    HashCombine(Seed, std::hash<std::string>{}(Desc.PSEntry));
    HashCombine(Seed, static_cast<size_t>(Desc.ShaderModel));
    HashDefines(Seed, Desc.Defines);

    for (DXGI_FORMAT Format : Desc.RTVFormats)
        HashCombine(Seed, static_cast<size_t>(Format));
    HashCombine(Seed, static_cast<size_t>(Desc.DSVFormat));

    for (const D3D12_INPUT_ELEMENT_DESC& E : Desc.InputLayout)
    {
        HashCombine(Seed, std::hash<std::string>{}(E.SemanticName ? E.SemanticName : ""));
        HashCombine(Seed, static_cast<size_t>(E.SemanticIndex));
        HashCombine(Seed, static_cast<size_t>(E.Format));
        HashCombine(Seed, static_cast<size_t>(E.InputSlot));
        HashCombine(Seed, static_cast<size_t>(E.AlignedByteOffset));
        HashCombine(Seed, static_cast<size_t>(E.InputSlotClass));
        HashCombine(Seed, static_cast<size_t>(E.InstanceDataStepRate));
    }

    HashCombine(Seed, static_cast<size_t>(Desc.CullMode));
    HashCombine(Seed, static_cast<size_t>(Desc.FillMode));
    HashCombine(Seed, static_cast<size_t>(Desc.bFrontCounterClockwise));
    HashCombine(Seed, static_cast<size_t>(Desc.DepthBias));
    HashCombine(Seed, std::hash<float>{}(Desc.SlopeScaledDepthBias));
    HashCombine(Seed, std::hash<float>{}(Desc.DepthBiasClamp));
    HashCombine(Seed, static_cast<size_t>(Desc.bDepthClipEnable));
    HashCombine(Seed, static_cast<size_t>(Desc.DepthTest));
    HashCombine(Seed, static_cast<size_t>(Desc.bDepthWrite));
    HashCombine(Seed, static_cast<size_t>(Desc.BlendMode));
    HashCombine(Seed, static_cast<size_t>(Desc.TopologyType));
    HashCombine(Seed, static_cast<size_t>(Desc.SampleCount));
    HashCombine(Seed, reinterpret_cast<size_t>(Desc.RootSignature));

    return Seed;
}

size_t FPipelineStateCache::MakeHash(const FComputePSODesc& Desc)
{
    size_t Seed = 0x517CC1B7;

    HashCombine(Seed, std::hash<std::wstring>{}(Desc.ShaderPath));
    HashCombine(Seed, std::hash<std::string>{}(Desc.CSEntry));
    HashCombine(Seed, static_cast<size_t>(Desc.ShaderModel));
    HashDefines(Seed, Desc.Defines);
    HashCombine(Seed, reinterpret_cast<size_t>(Desc.RootSignature));

    return Seed;
}

// ---------------------------------------------------------------------------
// Enum -> D3D12
// ---------------------------------------------------------------------------
static std::vector<FShaderMacro> ToShaderMacros(const std::vector<FShaderDefine>& Defines)
  {
      std::vector<FShaderMacro> Macros;
      Macros.reserve(Defines.size());
      for (const FShaderDefine& Define : Defines)
      {
          Macros.push_back(FShaderMacro::CreateShaderMacro(Define.Name.c_str(), "%s", Define.Value.c_str()));
      }
      return Macros;
  }

  static D3D12_CULL_MODE ToD3D12CullMode(ECullMode Mode)
  {
      switch (Mode)
      {
      case ECullMode::Front: return D3D12_CULL_MODE_FRONT;
      case ECullMode::Back:  return D3D12_CULL_MODE_BACK;
      default:               return D3D12_CULL_MODE_NONE;
      }
  }

  static D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(EDepthTest Test)
  {
      switch (Test)
      {
      case EDepthTest::Never:        return D3D12_COMPARISON_FUNC_NEVER;
      case EDepthTest::Less:         return D3D12_COMPARISON_FUNC_LESS;
      case EDepthTest::LessEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
      case EDepthTest::Greater:      return D3D12_COMPARISON_FUNC_GREATER;
      case EDepthTest::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
      case EDepthTest::Equal:        return D3D12_COMPARISON_FUNC_EQUAL;
      default:                       return D3D12_COMPARISON_FUNC_ALWAYS;
      }
  }

  static D3D12_RENDER_TARGET_BLEND_DESC MakeBlendDesc(EBlendMode Mode)
  {
      D3D12_RENDER_TARGET_BLEND_DESC Desc = {};
      Desc.LogicOpEnable        = FALSE;
      Desc.LogicOp              = D3D12_LOGIC_OP_NOOP;
      Desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
      Desc.BlendOp              = D3D12_BLEND_OP_ADD;
      Desc.BlendOpAlpha         = D3D12_BLEND_OP_ADD;

      switch (Mode)
      {
      case EBlendMode::AlphaBlend:
          Desc.BlendEnable    = TRUE;
          Desc.SrcBlend       = D3D12_BLEND_SRC_ALPHA;
          Desc.DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
          Desc.SrcBlendAlpha  = D3D12_BLEND_ONE;
          Desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
          break;

      case EBlendMode::Additive:
          Desc.BlendEnable    = TRUE;
          Desc.SrcBlend       = D3D12_BLEND_ONE;
          Desc.DestBlend      = D3D12_BLEND_ONE;
          Desc.SrcBlendAlpha  = D3D12_BLEND_ONE;
          Desc.DestBlendAlpha = D3D12_BLEND_ONE;
          break;

      case EBlendMode::PremultipliedAlpha:
          Desc.BlendEnable    = TRUE;
          Desc.SrcBlend       = D3D12_BLEND_ONE;
          Desc.DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
          Desc.SrcBlendAlpha  = D3D12_BLEND_ONE;
          Desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
          break;

      case EBlendMode::Opaque:
      default:
          Desc.BlendEnable    = FALSE;
          Desc.SrcBlend       = D3D12_BLEND_ONE;
          Desc.DestBlend      = D3D12_BLEND_ZERO;
          Desc.SrcBlendAlpha  = D3D12_BLEND_ONE;
          Desc.DestBlendAlpha = D3D12_BLEND_ZERO;
          break;
      }
      return Desc;
  }

FPipelineState* FPipelineStateCache::GetOrCreate(const FGraphicsPSODesc& Desc)
{
    const size_t Hash = MakeHash(Desc);
    if (auto It = mCache.find(Hash); It != mCache.end()) { return It->second.get(); }

    const std::vector<FShaderMacro> Macros = ToShaderMacros(Desc.Defines);

    FShaderStageCompileDesc VSDesc;
    VSDesc.FilePath    = Desc.ShaderPath;
    VSDesc.EntryPoint  = Desc.VSEntry;
    VSDesc.ShaderStage = EShaderStage::VertexShader;
    VSDesc.ShaderModel = Desc.ShaderModel;
    VSDesc.Macros      = Macros;

    ShaderUtils::FBlob* VS = FShaderManager::GetShader(VSDesc);
    if (!VS)
    {
        LUMINA_LOG_ERROR(RHI, "FPipelineStateCache: VS Compile Failed '%s'", StringUtils::WideToUTF8(Desc.ShaderPath).c_str());
        return nullptr;
    }

    ShaderUtils::FBlob* PS = nullptr;
    if (!Desc.PSEntry.empty())
    {
        FShaderStageCompileDesc PSDesc;
        PSDesc.FilePath    = Desc.ShaderPath;
        PSDesc.EntryPoint  = Desc.PSEntry;
        PSDesc.ShaderStage = EShaderStage::PixelShader;
        PSDesc.ShaderModel = Desc.ShaderModel;
        PSDesc.Macros      = Macros;

        PS = FShaderManager::GetShader(PSDesc);
        if (!PS)
        {
            LUMINA_LOG_ERROR(RHI, "FPipelineStateCache: PS Compile Failed '%s'", StringUtils::WideToUTF8(Desc.ShaderPath).c_str());
            return nullptr;
        }
    }

    FGraphicsPipelineStateBuilder Builder;
    Builder.SetRootSignature(Desc.RootSignature)
           .SetInputLayout(Desc.InputLayout)
           .SetRenderTargetFormats(Desc.RTVFormats, Desc.DSVFormat)
           .SetPrimitiveTopologyType(Desc.TopologyType)
           .SetVertexShader(VS->GetByteCode(), VS->GetByteCodeSize());

    if (PS) Builder.SetPixelShader(PS->GetByteCode(), PS->GetByteCodeSize());

    // Rasterizer
    D3D12_RASTERIZER_DESC RasterDesc = {};
    RasterDesc.FillMode = (Desc.FillMode == EFillMode::Wireframe)
                                ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    RasterDesc.CullMode = ToD3D12CullMode(Desc.CullMode);
    RasterDesc.FrontCounterClockwise = Desc.bFrontCounterClockwise;
    RasterDesc.DepthBias = Desc.DepthBias;
    RasterDesc.DepthBiasClamp = Desc.DepthBiasClamp;
    RasterDesc.SlopeScaledDepthBias = Desc.SlopeScaledDepthBias;
    RasterDesc.DepthClipEnable = Desc.bDepthClipEnable;
    RasterDesc.MultisampleEnable = (Desc.SampleCount > 1);
    RasterDesc.AntialiasedLineEnable = FALSE;
    RasterDesc.ForcedSampleCount = 0;
    RasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    Builder.SetRasterizeState(RasterDesc);

    // Depth / Stencil
    D3D12_DEPTH_STENCIL_DESC DepthDesc = {};
    DepthDesc.DepthEnable = (Desc.DepthTest != EDepthTest::Disabled);
    DepthDesc.DepthWriteMask = Desc.bDepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    DepthDesc.DepthFunc = ToD3D12ComparisonFunc(Desc.DepthTest);
    DepthDesc.StencilEnable = FALSE;
    Builder.SetDepthStencilState(DepthDesc);

    // Blend
    D3D12_BLEND_DESC BlendDesc = {};
    BlendDesc.AlphaToCoverageEnable = FALSE;
    BlendDesc.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC RTBlend = MakeBlendDesc(Desc.BlendMode);
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
    {
        BlendDesc.RenderTarget[i] = RTBlend;
    }
    Builder.SetBlendState(BlendDesc);

    auto PSO = std::make_unique<FPipelineState>();
    if (!Builder.Build(mpDevice->GetDevice(), *PSO)) return nullptr;

    FPipelineState* Result = PSO.get();
    mCache.emplace(Hash, std::move(PSO));
    return Result;
}

FPipelineState * FPipelineStateCache::GetOrCreate(const FComputePSODesc &Desc)
{
    const size_t Hash = MakeHash(Desc);
    if (auto it = mCache.find(Hash); it != mCache.end()) return it->second.get();

    FShaderStageCompileDesc CSDesc;
    CSDesc.FilePath    = Desc.ShaderPath;
    CSDesc.EntryPoint  = Desc.CSEntry;
    CSDesc.ShaderStage = EShaderStage::ComputeShader;
    CSDesc.ShaderModel = Desc.ShaderModel;
    CSDesc.Macros      = ToShaderMacros(Desc.Defines);

    ShaderUtils::FBlob* CS = FShaderManager::GetShader(CSDesc);
    if (!CS)
    {
        LUMINA_LOG_ERROR(RHI, "FPipelineStateCache: CS Compile Fail '%s'",
                         StringUtils::WideToUTF8(Desc.ShaderPath).c_str());
        return nullptr;
    }

    FComputePipelineStateBuilder Builder;
    Builder.SetRootSignature(Desc.RootSignature)
           .SetComputeShader(CS->GetByteCode(), CS->GetByteCodeSize());

    auto PSO = std::make_unique<FPipelineState>();
    if (!Builder.Build(mpDevice->GetDevice(), *PSO)) return nullptr;

    FPipelineState* Result = PSO.get();
    mCache.emplace(Hash, std::move(PSO));
    return Result;
}