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

#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/D3D12Core/Pipeline/Shader.h"

#include <d3d12.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class FDevice;

struct FGraphicsPSODesc
{
    // Shaders (single HLSL file with VS/PS entry points, matching current usage)
    std::wstring ShaderPath;
    std::string  VSEntry;
    std::string  PSEntry;
    EShaderModel ShaderModel = EShaderModel::SM6_0;

    // Fixed-function state that affects PSO identity
    std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
    std::vector<DXGI_FORMAT>              RTVFormats;
    DXGI_FORMAT                           DSVFormat   = DXGI_FORMAT_UNKNOWN;
    bool                                  bDepthTest  = false;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE         TopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // Root signature this PSO is built against (not hashed by pointer alone —
    // see note in MakeHash; for a single global root sig this is constant).
    ID3D12RootSignature* RootSignature = nullptr;
};

class FPipelineStateCache
{
public:
    static void Initialize(FDevice* pDevice);
    static void Shutdown();

    // Returns a cached PSO, building it on first request. nullptr on failure.
    static FPipelineState* GetOrCreate(const FGraphicsPSODesc& Desc);

private:
    static size_t MakeHash(const FGraphicsPSODesc& Desc);

    static FDevice* mpDevice;
    static std::unordered_map<size_t, std::unique_ptr<FPipelineState>> mCache;
};