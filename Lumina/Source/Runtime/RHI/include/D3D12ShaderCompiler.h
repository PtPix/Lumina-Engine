/**
 * @file D3D12ShaderCompiler.h
 * @brief DxcAPI Shader Compiler Wrapper.
 *
 * Compiles HLSL source code into DXIL using Microsoft's DirectX Shader Compiler (DXC).
 */

#pragma once

#include "D3D12Shader.h"

#include <string>
#include <vector>
#include <wrl/client.h>
#include <dxcapi.h>
#include <d3dcommon.h>

struct ID3D12ShaderReflection;

struct FD3D12ShaderStageCompileDesc;

namespace ShaderUtils
{
    struct FD3D12Blob
    {
    	FD3D12Blob() = default;
    	~FD3D12Blob() = default;

    	FD3D12Blob(FD3D12Blob&& Other) noexcept = default;
    	FD3D12Blob& operator=(FD3D12Blob&& Other) noexcept = default;

    	FD3D12Blob(const FD3D12Blob&) = delete;
    	FD3D12Blob& operator=(const FD3D12Blob&) = delete;

        [[nodiscard]] bool IsNull() const;
        [[nodiscard]] const void* GetByteCode() const;
        [[nodiscard]] size_t GetByteCodeSize() const;

        Microsoft::WRL::ComPtr<ID3DBlob> pD3DBlob = nullptr;
        Microsoft::WRL::ComPtr<IDxcBlob> pDxcBlob = nullptr;
    };

    union FD3D12ShaderBlobs
    {
        struct
        {
            FD3D12Blob* VertexShaderBlobs;
            FD3D12Blob* GeometryShaderBlobs;
            FD3D12Blob* DomainShaderBlobs;
            FD3D12Blob* HullShaderBlobs;
            FD3D12Blob* PixelShaderBlobs;
            FD3D12Blob* ComputeShaderBlobs;
        };
        FD3D12Blob* Blobs[EShaderStage::NUM_SHADER_STAGES];
    };

    union FD3D12ShaderReflections
    {
        struct
        {
            ID3D12ShaderReflection* VertexShaderReflection;
            ID3D12ShaderReflection* GeometryShaderReflection;
            ID3D12ShaderReflection* DomainShaderReflection;
            ID3D12ShaderReflection* HullShaderReflection;
            ID3D12ShaderReflection* PixelShaderReflection;
            ID3D12ShaderReflection* ComputeShaderReflection;
        };
        ID3D12ShaderReflection* Reflections[EShaderStage::NUM_SHADER_STAGES] = { nullptr };
    };

	// TODO : a static compiler class is needed.
	FD3D12Blob CompileFromSource(const FD3D12ShaderStageCompileDesc& ShaderStageCompileDesc, std::string& OutErrorString);
	bool CompileFromCachedBinary(const std::string& ShaderBinaryFilePath, FD3D12Blob& Blob, bool bSM6, std::string& ErrorMsg);
	void CacheShaderBinary(const std::string& ShaderBinaryFilePath, size_t ShaderBinarySize, const void* pShaderBinary);

	const char* GetShaderModel_cstr(const EShaderModel& ShaderModel, const EShaderStage ShaderStage);
	const wchar_t* GetShaderModel_wcstr(const EShaderModel& ShaderModel, const EShaderStage ShaderStage);
}

struct FD3D12ShaderStageCompileDesc
{
    std::wstring FilePath;
    std::string EntryPoint;
    EShaderStage ShaderStage = EShaderStage::NUM_SHADER_STAGES;
    EShaderModel ShaderModel = EShaderModel::SM6_0;
    std::vector<FShaderMacro> Macros;
    bool bUseNative16bit = false;
    std::vector<std::wstring> DXCompilerFlags;
};

struct FD3D12ShaderStageCompileResult
{
    ShaderUtils::FD3D12Blob ShaderBlob;
	EShaderStage ShaderStage;
	std::wstring FilePath;
	bool bSM6;
};
