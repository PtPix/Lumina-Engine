#include "Shaders/ShaderManager.h"
#include "D3D12Shader.h"
#include "D3D12ShaderCompiler.h"
#include "Logger/Logger.h"
#include "StringUtils/StringConv.h"

std::unordered_map<std::string, std::unique_ptr<ShaderUtils::FD3D12Blob>> FShaderManager::mShaderCache;

ShaderUtils::FD3D12Blob* FShaderManager::GetShader(const FD3D12ShaderStageCompileDesc& Desc)
{
    const std::string Key = MakeKey(Desc);

    if (auto It = mShaderCache.find(Key); It != mShaderCache.end())
    {
        return It->second.get();
    }

    std::string ErrorString;
    ShaderUtils::FD3D12Blob Blob = ShaderUtils::CompileFromSource(Desc, ErrorString);
    if (Blob.IsNull())
    {
        LUMINA_LOG_ERROR(RHI, "FShaderManager: failed to compile '%s' entry '%s': %s",
                         StringUtils::WideToUTF8(Desc.FilePath).c_str(),
                         Desc.EntryPoint.c_str(), ErrorString.c_str());
        return nullptr;
    }

    auto Stored = std::make_unique<ShaderUtils::FD3D12Blob>(std::move(Blob));
    ShaderUtils::FD3D12Blob* Result = Stored.get();
    mShaderCache.emplace(Key, std::move(Stored));
    return Result;
}

void FShaderManager::Clear()
{
    mShaderCache.clear();
}

std::string FShaderManager::MakeKey(const FD3D12ShaderStageCompileDesc& Desc)
{
    // path | entry | stage | model | macros
    std::string Key = StringUtils::WideToUTF8(Desc.FilePath);
    Key += '|';
    Key += Desc.EntryPoint;
    Key += '|';
    Key += std::to_string(static_cast<uint32_t>(Desc.ShaderStage));
    Key += '|';
    Key += std::to_string(static_cast<uint32_t>(Desc.ShaderModel));
    for (const FShaderMacro& Macro : Desc.Macros)
    {
        Key += '|';
        Key += Macro.Name;
        Key += '=';
        Key += Macro.Value;
    }
    return Key;
}
