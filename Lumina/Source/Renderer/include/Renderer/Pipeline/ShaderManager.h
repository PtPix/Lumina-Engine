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

#include "../D3D12/D3D12ShaderCompiler.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <d3d12shader.h>

class FShaderManager
{
public:
    // Returns a cached blob, compiling on first request.
    // Returns nullptr (and logs) if compilation fails.
    static ShaderUtils::FD3D12Blob* GetShader(const FD3D12ShaderStageCompileDesc& Desc);

    static void Clear();

private:
    static std::string MakeKey(const FD3D12ShaderStageCompileDesc& Desc);

    static std::unordered_map<std::string, std::unique_ptr<ShaderUtils::FD3D12Blob>> mShaderCache;
};