/**
 * @file PipelineStateCache.h
 * @brief Hash-keyed graphics PSO cache.
 *
 * A pass describes the PSO it needs via FGraphicsPSODesc and calls GetOrCreate.
 * On a cache miss the shaders are fetched (FShaderManager) and the PSO is built
 * (FGraphicsPipelineStateBuilder); on a hit the cached PSO is returned.
 * Ownership of every PSO lives here, not in individual passes.
 */

#pragma once

#include "Renderer/D3D12/D3D12PipelineState.h"
#include "Renderer/D3D12/D3D12Shader.h"
#include "Renderer/Core/RenderTypes.h"

#include <d3d12.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class FD3D12Device;

// Shader permutation
struct FShaderDefine
{
    std::string Name;
    std::string Value;

    FShaderDefine() = default;
    FShaderDefine(std::string InName, std::string InValue) : Name(std::move(InName)), Value(std::move(InValue)) {}
};

// Type Enum
enum class ECullMode : uint8_t
{
    None,
    Front,
    Back,
};

enum class EDepthTest : uint8_t
{
    Disabled,
    Never,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    Always,
};

enum class EFillMode : uint8_t
{
    Solid,
    Wireframe,
};

struct FGraphicsPSODesc
{
    // Shaders
    std::wstring ShaderPath;
    std::string  VSEntry;
    std::string  PSEntry;
    EShaderModel ShaderModel = EShaderModel::SM6_0;
    std::vector<FShaderDefine> Defines;

    // Input / Output
    std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
    std::vector<DXGI_FORMAT>              RTVFormats;
    DXGI_FORMAT                           DSVFormat   = DXGI_FORMAT_UNKNOWN;

    // Rasterizer
    ECullMode CullMode = ECullMode::Back;
    EFillMode FillMode = EFillMode::Solid;
    bool bFrontCounterClockwise = false;
    int32_t DepthBias = 0;
    float SlopeScaledDepthBias = 0.0f;
    float DepthBiasClamp = 0.0f;
    bool bDepthClipEnable = true;

    // Depth / Stencil
    EDepthTest DepthTest  = EDepthTest::Disabled;
    bool bDepthWrite = true;

    // Blend
    EBlendMode BlendMode = EBlendMode::Opaque;

    D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    uint32_t SampleCount = 1;

    ID3D12RootSignature* RootSignature = nullptr;
};

struct FComputePSODesc
{
    std::wstring ShaderPath;
    std::string CSEntry;
    EShaderModel ShaderModel = EShaderModel::SM6_0;
    std::vector<FShaderDefine> Defines;

    ID3D12RootSignature* RootSignature = nullptr;
};

class FPipelineStateCache
{
public:
    static void Initialize(FD3D12Device* pDevice);
    static void Shutdown();

    static FD3D12PipelineState* GetOrCreate(const FGraphicsPSODesc& Desc);
    static FD3D12PipelineState* GetOrCreate(const FComputePSODesc& Desc);

    static void InvalidateAll();

    [[nodiscard]] static size_t GetCachedCount();

private:
    static size_t MakeHash(const FGraphicsPSODesc& Desc);
    static size_t MakeHash(const FComputePSODesc& Desc);

    static FD3D12Device* mpDevice;
    static std::unordered_map<size_t, std::unique_ptr<FD3D12PipelineState>> mCache;
};