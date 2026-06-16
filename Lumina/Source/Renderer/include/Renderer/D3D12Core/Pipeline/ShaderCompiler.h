/**
 * @file ShaderCompiler.h
 * @brief DxcAPI Shader Compiler Wrapper.
 *
 * Compiles HLSL source code into DXIL using Microsoft's DirectX Shader Compiler (DXC).
 */

#pragma once

#include "Renderer/D3D12Core/Pipeline/Shader.h"

#include <string>
#include <vector>
#include <wrl/client.h>
#include <dxcapi.h>

struct ID3D12ShaderReflection;
struct ID3D10Blob;

struct FShaderStageCompileDesc;

namespace ShaderUtils
{
    struct FBlob
    {
    	FBlob() = default;
    	~FBlob() = default;

    	FBlob(FBlob&& Other) noexcept = default;
    	FBlob& operator=(FBlob&& Other) noexcept = default;

    	FBlob(const FBlob&) = delete;
    	FBlob& operator=(const FBlob&) = delete;

        [[nodiscard]] bool IsNull() const;
        [[nodiscard]] const void* GetByteCode() const;
        [[nodiscard]] size_t GetByteCodeSize() const;

        Microsoft::WRL::ComPtr<ID3D10Blob> pD3DBlob = nullptr;
        Microsoft::WRL::ComPtr<IDxcBlob> pDxcBlob = nullptr;
    };

    union ShaderBlobs
    {
        struct
        {
            FBlob* VertexShaderBlobs;
            FBlob* GeometryShaderBlobs;
            FBlob* DomainShaderBlobs;
            FBlob* HullShaderBlobs;
            FBlob* PixelShaderBlobs;
            FBlob* ComputeShaderBlobs;
        };
        FBlob* Blobs[EShaderStage::NUM_SHADER_STAGES];
    };

    union ShaderReflections
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
	FBlob CompileFromSource(const FShaderStageCompileDesc& ShaderStageCompileDesc, std::string& OutErrorString);
	bool CompileFromCachedBinary(const std::string& ShaderBinaryFilePath, FBlob& Blob, bool bSM6, std::string& ErrorMsg);
	void CacheShaderBinary(const std::string& ShaderBinaryFilePath, size_t ShaderBinarySize, const void* pShaderBinary);

	const char* GetShaderModel_cstr(const EShaderModel& ShaderModel, const EShaderStage ShaderStage);
	const wchar_t* GetShaderModel_wcstr(const EShaderModel& ShaderModel, const EShaderStage ShaderStage);
}

struct FShaderStageCompileDesc
{
    std::wstring FilePath;
    std::string EntryPoint;
    EShaderStage ShaderStage = EShaderStage::NUM_SHADER_STAGES;
    EShaderModel ShaderModel = EShaderModel::SM6_0;
    std::vector<FShaderMacro> Macros;
    bool bUseNative16bit = false;
    std::vector<std::wstring> DXCompilerFlags;
};

struct FShaderStageCompileResult
{
    ShaderUtils::FBlob ShaderBlob;
	EShaderStage ShaderStage;
	std::wstring FilePath;
	bool bSM6;
};
