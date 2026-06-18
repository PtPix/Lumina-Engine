#include "Renderer/Pipeline/ShaderManager.h"

std::unordered_map<std::string, std::unique_ptr<ShaderUtils::FBlob>> FShaderManager::mShaderCache;

ShaderUtils::FBlob* FShaderManager::GetShader(const FShaderStageCompileDesc& Desc)
{
    const std::string Key = MakeKey(Desc);

    if (auto It = mShaderCache.find(Key); It != mShaderCache.end())
    {
        return It->second.get();
    }

    std::string ErrorString;
    ShaderUtils::FBlob Blob = ShaderUtils::CompileFromSource(Desc, ErrorString);
    if (Blob.IsNull())
    {
        LUMINA_LOG_ERROR(RHI, "FShaderManager: failed to compile '%s' entry '%s': %s",
                         StringUtils::WideToUTF8(Desc.FilePath).c_str(),
                         Desc.EntryPoint.c_str(), ErrorString.c_str());
        return nullptr;
    }

    auto Stored = std::make_unique<ShaderUtils::FBlob>(std::move(Blob));
    ShaderUtils::FBlob* Result = Stored.get();
    mShaderCache.emplace(Key, std::move(Stored));
    return Result;
}

void FShaderManager::Clear()
{
    mShaderCache.clear();
}

std::string FShaderManager::MakeKey(const FShaderStageCompileDesc& Desc)
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
