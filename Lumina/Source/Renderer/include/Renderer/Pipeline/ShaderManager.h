/**
 * @file ShaderManager.h
 * @brief Global shader compilation cache.
 *
 * Compiles HLSL on first request and caches the resulting blob, keyed by
 * (path, entry point, stage, model, macros). Subsequent requests for the
 * same key return the cached blob. Replaces ad-hoc CompileFromSource calls
 * scattered in passes.
 */

#pragma once

#include "Renderer/D3D12Core/Pipeline/ShaderCompiler.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <d3d12shader.h>

class FShaderManager
{
public:
    // Returns a cached blob, compiling on first request.
    // Returns nullptr (and logs) if compilation fails.
    static ShaderUtils::FBlob* GetShader(const FShaderStageCompileDesc& Desc);

    static void Clear();

private:
    static std::string MakeKey(const FShaderStageCompileDesc& Desc);

    static std::unordered_map<std::string, std::unique_ptr<ShaderUtils::FBlob>> mShaderCache;
};