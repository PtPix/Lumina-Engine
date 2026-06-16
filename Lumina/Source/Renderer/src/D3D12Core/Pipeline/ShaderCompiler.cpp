#include "Renderer/D3D12Core/Pipeline/ShaderCompiler.h"
#include "Renderer/D3D12Core/Common.h"

#include <dxcapi.h>
#include <d3dcompiler.h>

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <fstream>

const UINT SHADER_COMPILE_FLAGS = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_DEBUG_NAME_FOR_BINARY;

static const std::unordered_map<UINT, LPCWSTR> D3DCompilerFlagCompatibilityLookup =
{
    { D3DCOMPILE_DEBUG                         , DXC_ARG_DEBUG }
    ,{ D3DCOMPILE_SKIP_VALIDATION               , DXC_ARG_SKIP_VALIDATION }
    ,{ D3DCOMPILE_SKIP_OPTIMIZATION             , DXC_ARG_SKIP_OPTIMIZATIONS }
    ,{ D3DCOMPILE_PACK_MATRIX_ROW_MAJOR         , DXC_ARG_PACK_MATRIX_ROW_MAJOR }
    ,{ D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR      , DXC_ARG_PACK_MATRIX_COLUMN_MAJOR }
    ,{ D3DCOMPILE_AVOID_FLOW_CONTROL            , DXC_ARG_AVOID_FLOW_CONTROL }
    ,{ D3DCOMPILE_PREFER_FLOW_CONTROL           , DXC_ARG_PREFER_FLOW_CONTROL }
    ,{ D3DCOMPILE_ENABLE_STRICTNESS             , DXC_ARG_ENABLE_STRICTNESS }
    ,{ D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, DXC_ARG_ENABLE_BACKWARDS_COMPATIBILITY }
    ,{ D3DCOMPILE_IEEE_STRICTNESS               , DXC_ARG_IEEE_STRICTNESS }
    ,{ D3DCOMPILE_OPTIMIZATION_LEVEL0           , DXC_ARG_OPTIMIZATION_LEVEL0 }
    ,{ D3DCOMPILE_OPTIMIZATION_LEVEL1           , DXC_ARG_OPTIMIZATION_LEVEL1 }
    ,{ D3DCOMPILE_OPTIMIZATION_LEVEL2           , DXC_ARG_OPTIMIZATION_LEVEL2 }
    ,{ D3DCOMPILE_OPTIMIZATION_LEVEL3           , DXC_ARG_OPTIMIZATION_LEVEL3 }
    ,{ D3DCOMPILE_WARNINGS_ARE_ERRORS           , DXC_ARG_WARNINGS_ARE_ERRORS }
    ,{ D3DCOMPILE_RESOURCES_MAY_ALIAS           , DXC_ARG_RESOURCES_MAY_ALIAS }
    ,{ D3DCOMPILE_ALL_RESOURCES_BOUND           , DXC_ARG_ALL_RESOURCES_BOUND }
    ,{ D3DCOMPILE_DEBUG_NAME_FOR_SOURCE         , DXC_ARG_DEBUG_NAME_FOR_SOURCE }
    ,{ D3DCOMPILE_DEBUG_NAME_FOR_BINARY         , DXC_ARG_DEBUG_NAME_FOR_BINARY }
};

bool ShaderUtils::FBlob::IsNull() const
{
    return !pD3DBlob && !pDxcBlob;
}

const void* ShaderUtils::FBlob::GetByteCode() const
{
    if (this->pD3DBlob)
        return this->pD3DBlob->GetBufferPointer();
    if (this->pDxcBlob)
        return this->pDxcBlob->GetBufferPointer();

    assert(!IsNull());
    return nullptr;
}

size_t ShaderUtils::FBlob::GetByteCodeSize() const
{
    if (this->pD3DBlob)
        return this->pD3DBlob->GetBufferSize();
    if (this->pDxcBlob)
        return this->pDxcBlob->GetBufferSize();
    return 0;
}

const char* ShaderUtils::GetShaderModel_cstr(const EShaderModel& ShaderModel, const EShaderStage ShaderStage)
{
    static const char* ShaderModelStrings[][EShaderStage::NUM_SHADER_STAGES] = {
        { "vs_5_0", "gs_5_0", "ds_5_0", "hs_5_0", "ps_5_0", "cs_5_0" },
        { "vs_6_0", "gs_6_0", "ds_6_0", "hs_6_0", "ps_6_0", "cs_6_0" },
        { "vs_6_1", "gs_6_1", "ds_6_1", "hs_6_1", "ps_6_1", "cs_6_1" },
        { "vs_6_2", "gs_6_2", "ds_6_2", "hs_6_2", "ps_6_2", "cs_6_2" },
        { "vs_6_3", "gs_6_3", "ds_6_3", "hs_6_3", "ps_6_3", "cs_6_3" },
        { "vs_6_4", "gs_6_4", "ds_6_4", "hs_6_4", "ps_6_4", "cs_6_4" },
        { "vs_6_5", "gs_6_5", "ds_6_5", "hs_6_5", "ps_6_5", "cs_6_5" },
        { "vs_6_6", "gs_6_6", "ds_6_6", "hs_6_6", "ps_6_6", "cs_6_6" },
        { "vs_6_7", "gs_6_7", "ds_6_7", "hs_6_7", "ps_6_7", "cs_6_7" },
        { "vs_6_8", "gs_6_8", "ds_6_8", "hs_6_8", "ps_6_8", "cs_6_8" },
    };
    if (ShaderStage < EShaderStage::NUM_SHADER_STAGES && ShaderModel < EShaderModel::NUM_SHADER_MODELS)
    {
        return ShaderModelStrings[ShaderModel][ShaderStage];
    }
    return nullptr;
}

const wchar_t* ShaderUtils::GetShaderModel_wcstr(const EShaderModel& ShaderModel, const EShaderStage ShaderStage)
{
    static const wchar_t* ShaderModelStrings[][EShaderStage::NUM_SHADER_STAGES] = {
        { L"vs_5_0", L"gs_5_0", L"ds_5_0", L"hs_5_0", L"ps_5_0", L"cs_5_0" },
        { L"vs_6_0", L"gs_6_0", L"ds_6_0", L"hs_6_0", L"ps_6_0", L"cs_6_0" },
        { L"vs_6_1", L"gs_6_1", L"ds_6_1", L"hs_6_1", L"ps_6_1", L"cs_6_1" },
        { L"vs_6_2", L"gs_6_2", L"ds_6_2", L"hs_6_2", L"ps_6_2", L"cs_6_2" },
        { L"vs_6_3", L"gs_6_3", L"ds_6_3", L"hs_6_3", L"ps_6_3", L"cs_6_3" },
        { L"vs_6_4", L"gs_6_4", L"ds_6_4", L"hs_6_4", L"ps_6_4", L"cs_6_4" },
        { L"vs_6_5", L"gs_6_5", L"ds_6_5", L"hs_6_5", L"ps_6_5", L"cs_6_5" },
        { L"vs_6_6", L"gs_6_6", L"ds_6_6", L"hs_6_6", L"ps_6_6", L"cs_6_6" },
        { L"vs_6_7", L"gs_6_7", L"ds_6_7", L"hs_6_7", L"ps_6_7", L"cs_6_7" },
        { L"vs_6_8", L"gs_6_8", L"ds_6_8", L"hs_6_8", L"ps_6_8", L"cs_6_8" },
    };
    if (ShaderStage < EShaderStage::NUM_SHADER_STAGES && ShaderModel < EShaderModel::NUM_SHADER_MODELS)
    {
        return ShaderModelStrings[ShaderModel][ShaderStage];
    }
    return nullptr;
}

ShaderUtils::FBlob ShaderUtils::CompileFromSource(const FShaderStageCompileDesc& ShaderStageCompileDesc,
                                                  std::string& OutErrorString)
{
    const bool bIsShaderModel5 = ShaderStageCompileDesc.ShaderModel == EShaderModel::SM5_0;

    LUMINA_LOG_INFO(Shader, "Compiling Shader Source: [%s @ %s()]"
    , GetShaderModel_cstr(ShaderStageCompileDesc.ShaderModel, ShaderStageCompileDesc.ShaderStage)
    , ShaderStageCompileDesc.EntryPoint.c_str()
    );

    FBlob Blob;

    // SM5
    if (bIsShaderModel5)
    {
        LUMINA_LOG_ERROR(Shader, "We don't support SM5.0 for now");
        return Blob;
    }

    // Setup File Paths using std::filesystem
    std::filesystem::path AbsolutePath = std::filesystem::absolute(ShaderStageCompileDesc.FilePath);
    std::wstring widePath              = AbsolutePath.wstring();
    std::wstring wideParentPath        = AbsolutePath.parent_path().wstring();

    // Prepare Compiler Arguments
    std::vector<LPCWSTR> ppArgs = {};

    const std::wstring StringEntryPoint = StringUtils::ASCIIToUnicode(ShaderStageCompileDesc.EntryPoint);
    std::vector<std::wstring> UnicodeDefineArgs;

    for (const FShaderMacro& Macro : ShaderStageCompileDesc.Macros)
    {
        UnicodeDefineArgs.push_back(StringUtils::ASCIIToUnicode(Macro.Name) + L"=" + StringUtils::ASCIIToUnicode(Macro.Value));
    }

    // Initialize DXC Compiler
    // TODO: Creating DXC Instances per-compile is extremely slow. Consider caching IDxcCompiler3 and IDxcUtils.
    Microsoft::WRL::ComPtr<IDxcCompiler3> DxcCompiler3;
    Microsoft::WRL::ComPtr<IDxcUtils>     DxcUtils;

    HRESULT HResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DxcCompiler3));
    assert(SUCCEEDED(HResult) && "Couldn't initialize DirectXCompiler");

    HResult = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&DxcUtils));
    assert(SUCCEEDED(HResult) && "Couldn't initialize DirectXCompiler Utils");

    // Read Shader Source File
    std::ifstream shaderFile(widePath, std::ios::binary | std::ios::ate);
    if (!shaderFile.is_open())
    {
        LUMINA_LOG_ERROR(Shader, "Failed to open shader file.");
        return Blob;
    }

    std::streamsize size = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);

    std::vector<char> shaderSource(size);
    if (!shaderFile.read(shaderSource.data(), size)) return Blob;

    Microsoft::WRL::ComPtr<IDxcBlobEncoding> SourceBlob;
    HResult = DxcUtils->CreateBlobFromPinned(shaderSource.data(), static_cast<uint32_t>(shaderSource.size()), CP_UTF8, &SourceBlob);
    assert(SUCCEEDED(HResult) && "Couldn't create SourceBlob");

    DxcBuffer SourceBuffer = {};
    SourceBuffer.Ptr      = SourceBlob->GetBufferPointer();
    SourceBuffer.Size     = SourceBlob->GetBufferSize();
    SourceBuffer.Encoding = DXC_CP_ACP;

    // Apply D3DCompile Equivalent Flags
    for (const auto& prFlag : D3DCompilerFlagCompatibilityLookup)
    {
        if (SHADER_COMPILE_FLAGS & prFlag.first)
        {
            ppArgs.push_back(prFlag.second);
            if (prFlag.first == D3DCOMPILE_DEBUG)
            {
                ppArgs.push_back(L"-Qembed_debug");
                ppArgs.push_back(L"-Zi");
            }
        }
    }

    if (ShaderStageCompileDesc.bUseNative16bit)
    {
        ppArgs.push_back(L"-enable-16bit-types");
    }

    // Append custom user compiler flags
    for (const std::wstring& Flag : ShaderStageCompileDesc.DXCompilerFlags)
    {
        ppArgs.push_back(Flag.c_str());
    }

    // Append Macros/Defines
    for (const std::wstring& UnicodeDefineArg : UnicodeDefineArgs)
    {
        ppArgs.push_back(L"-D");
        ppArgs.push_back(UnicodeDefineArg.c_str());
    }

    // Entry Point & Shader Model & Path
    ppArgs.push_back(L"-E"); ppArgs.push_back(StringEntryPoint.c_str());
    ppArgs.push_back(L"-T"); ppArgs.push_back(GetShaderModel_wcstr(ShaderStageCompileDesc.ShaderModel, ShaderStageCompileDesc.ShaderStage));
    ppArgs.push_back(L"-I"); ppArgs.push_back(wideParentPath.c_str()); // Include paths
    ppArgs.push_back(widePath.c_str());

    // Setup Include Handler
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
    DxcUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

    // Execute DXC Compilation
    Microsoft::WRL::ComPtr<IDxcResult> pResult;
    HResult = DxcCompiler3->Compile(&SourceBuffer, ppArgs.data(), static_cast<UINT32>(ppArgs.size()), pIncludeHandler.Get(), IID_PPV_ARGS(&pResult));

    if (SUCCEEDED(HResult))
    {
        pResult->GetStatus(&HResult);
    }

    // Handle Errors and Warnings
    Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
    if (SUCCEEDED(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)) && pErrors && pErrors->GetStringLength() > 0)
    {
        OutErrorString = pErrors->GetStringPointer();
        if (FAILED(HResult))
        {
            LUMINA_LOG_ERROR(Shader, "Shader Compilation Error:\n%s", OutErrorString.c_str());
        }
        else
        {
            LUMINA_LOG_WARNING(Shader, "Shader Compilation Warning:\n%s", OutErrorString.c_str());
        }
    }

    // Return empty blob on failure
    if (FAILED(HResult)) return Blob;

    // Retrieve Compiled DXIL Object
    HResult = pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&Blob.pDxcBlob), nullptr);

    return Blob;
}

bool ShaderUtils::CompileFromCachedBinary(const std::string& ShaderBinaryFilePath, FBlob& Blob, bool bSM6,
    std::string& ErrorMsg)
{
    return false;
}

void ShaderUtils::CacheShaderBinary(const std::string& ShaderBinaryFilePath, size_t ShaderBinarySize,
    const void* pShaderBinary)
{
}
