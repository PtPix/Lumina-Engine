#include "../../include/Renderer/D3D12Core/ShaderCompiler.h"
#include "dxcapi.h"
#include <algorithm>
#include <cassert>
#include <d3dcompiler.h>
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

template<unsigned STR_SIZE>
std::string UnicodeToASCII(const WCHAR wchars[STR_SIZE])
{
    char ascii[STR_SIZE];
    size_t numCharsConverted = 0;
    wcstombs_s(&numCharsConverted, ascii, wchars, STR_SIZE);
    return std::string(ascii);
}
std::vector<std::string> split(const char* s, char c)
{
    std::vector<std::string> result;
    do
    {
        const char* begin = s;

        if (*begin == c || *begin == '\0')
            continue;	// skip delimiter character

        while (*s != c && *s)
            s++;	// iterate until delimiter is found

        result.push_back(std::string(begin, s));

    } while (*s++);
    return result;
}

std::vector<std::string> split(const std::string& str, char c)
{
    return split(str.c_str(), c);
}

std::vector<std::string> split(std::string_view s, const std::vector<char>& delimiters)
{
    std::vector<std::string> result;
    const char* ps = s.data();
    auto IsDelimiter = [&delimiters](const char c)
    {
        return std::find(delimiters.begin(), delimiters.end(), c) != delimiters.end();
    };

    do
    {
        const char* begin = ps;

        if (IsDelimiter(*begin) || (*begin == '\0'))
            continue;	// skip delimiter characters

        while (!IsDelimiter(*ps) && *ps)
            ps++;	// iterate until delimiter is found or string has ended

        result.push_back(std::string(begin, ps));

    } while (*ps++);
    return result;
}
std::wstring ASCIIToUnicode(const std::string& str) { return std::wstring(str.begin(), str.end()); }
std::wstring ASCIIToUnicode(const char* str) { return std::wstring(str, str + strlen(str)); }
std::string GetFolderPath(const std::string & pathToFile)
{
    const auto tokens = split(pathToFile, { '/', '\\' });
    std::string path = "";
    if (!tokens.empty())
        for (int i = 0; i < tokens.size() - 1; ++i)
            path += tokens[i] + "/";
    return path;
}


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

ShaderUtils::FBlob::FBlob() = default;
ShaderUtils::FBlob::~FBlob() = default;
ShaderUtils::FBlob::FBlob(FBlob&& Other) noexcept = default;
ShaderUtils::FBlob& ShaderUtils::FBlob::operator=(FBlob&& Other) noexcept = default;

ShaderUtils::FBlob ShaderUtils::CompileFromSource(const FShaderStageCompileDesc& ShaderStageCompileDesc,
                                                  std::string& OutErrorString)
{
    const WCHAR* StringPath = ShaderStageCompileDesc.FilePath.data();
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
    }
    // SM6
    else
    {
        const std::wstring StringEntryPoint = ASCIIToUnicode(ShaderStageCompileDesc.EntryPoint);
        const std::wstring StringParentFolder = ASCIIToUnicode(GetFolderPath(UnicodeToASCII<260>(StringPath)));
        std::vector<std::wstring> UnicodeDefineArgs;
        for (const FShaderMacro& Marco : ShaderStageCompileDesc.Macros)
        {
            UnicodeDefineArgs.push_back(ASCIIToUnicode(Marco.Name) + L"=" + ASCIIToUnicode(Marco.Value));
        }
        std::vector<LPCWSTR> ppArgs = {};

        Microsoft::WRL::ComPtr<IDxcCompiler3> DxcCompiler3;
        Microsoft::WRL::ComPtr<IDxcUtils> DxcUtils;

        HRESULT HResult = {};
        HResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DxcCompiler3));
        if (FAILED(HResult))
        {
            LUMINA_LOG_ERROR(Shader, "Couldn't initialize DirectXCompiler Compiler");
            assert(false);
        }
        HResult = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&DxcUtils));
        if (FAILED(HResult))
        {
            LUMINA_LOG_ERROR(Shader, "Couldn't initialize DirectXCompiler Utils");
            assert(false);
        }

        std::filesystem::path FilePathFS(ShaderStageCompileDesc.FilePath);
        std::filesystem::path AbsolutePath = std::filesystem::absolute(FilePathFS);

        std::wstring widePath = AbsolutePath.wstring();
        std::wstring wideParentPath = AbsolutePath.parent_path().wstring();
        // Load Source file
        uint32_t CodePage = CP_UTF8;
        Microsoft::WRL::ComPtr<IDxcBlobEncoding> SourceBlob;
        std::ifstream shaderFile(widePath, std::ios::binary | std::ios::ate);
        if (!shaderFile.is_open()) return {};

        std::streamsize size = shaderFile.tellg();
        shaderFile.seekg(0, std::ios::beg);

        std::vector<char> shaderSource(size);
        if (!shaderFile.read(shaderSource.data(), size)) return {};
        Log::Info("Trying to load shader from absolute path: %s", AbsolutePath.string().c_str());
        HResult = DxcUtils->CreateBlobFromPinned(shaderSource.data(), (uint32_t)shaderSource.size(), CP_UTF8, &SourceBlob);
        if (FAILED(HResult))
        {
            LUMINA_LOG_ERROR(Shader, "Couldn't create SourceBlob");
            assert(false);
        }
        DxcBuffer SourceBuffer{};
        SourceBuffer.Ptr = SourceBlob->GetBufferPointer();
        SourceBuffer.Size = SourceBlob->GetBufferSize();
        SourceBuffer.Encoding = DXC_CP_ACP;

        // Build Args : Support Compile Flag from D3DCompile
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

        // Add NativeFP16
        if (ShaderStageCompileDesc.bUseNative16bit)
        {
            ppArgs.push_back(L"-enable-16bit-types");
        }

        // User-provided compiler flags
        for (const std::wstring& Flag : ShaderStageCompileDesc.DXCompilerFlags)
        {
            ppArgs.push_back(Flag.c_str());
        }

        // Defines
        for (const std::wstring& UnicodeDefineArg : UnicodeDefineArgs)
        {
            ppArgs.push_back(L"-D");
            ppArgs.push_back(UnicodeDefineArg.c_str());
        }

        // Enrty Point
        ppArgs.push_back(L"-E");
        ppArgs.push_back(StringEntryPoint.c_str());

        // Shader Model
        ppArgs.push_back(L"-T");
        ppArgs.push_back(GetShaderModel_wcstr(ShaderStageCompileDesc.ShaderModel, ShaderStageCompileDesc.ShaderStage));

        // Include Path
        ppArgs.push_back(L"-I");
        ppArgs.push_back(StringParentFolder.c_str());

        ppArgs.push_back(widePath.c_str());

        // Include Handler
        Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
        HResult = DxcUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

        // Compile
        Microsoft::WRL::ComPtr<IDxcResult> pResult;
        HResult = DxcCompiler3->Compile(&SourceBuffer, ppArgs.data(), (UINT32)ppArgs.size(),
            pIncludeHandler.Get(), IID_PPV_ARGS(&pResult));
        pResult->GetStatus(&HResult);

        Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
        HResult = pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
        if (pErrors != nullptr && pErrors->GetStringLength() != 0)
        {
            OutErrorString = pErrors->GetStringPointer();
            if (FAILED(HResult))
            {
                LUMINA_LOG_ERROR(Shader, OutErrorString.c_str());
            }
            else
            {
                LUMINA_LOG_WARNING(Shader, OutErrorString.c_str());
            }
        }

        if (FAILED(HResult))
        {
            return {};
        }

        Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
        HResult = pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&Blob.pDxcBlob), nullptr);
        if (FAILED(HResult))
        {
            return {};
        }
    }
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
