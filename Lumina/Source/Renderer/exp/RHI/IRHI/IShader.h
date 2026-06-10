#pragma once

#include <cstdint>
#include <string>

#include "Renderer/RHI/common/Resources.h"

namespace LuminaRHI
{
    enum class EShaderType : uint16_t
    {
        None            = 0x0000,

        Compute         = 0x0020,

        Vertex          = 0x0001,
        Hull            = 0x0002,
        Domain          = 0x0004,
        Geometry        = 0x0008,
        Pixel           = 0x0010,
        Amplification   = 0x0040,
        Mesh            = 0x0080,
        AllGraphics     = 0x00DF,

        RayGeneration   = 0x0100,
        AnyHit          = 0x0200,
        ClosestHit      = 0x0400,
        Miss            = 0x0800,
        Intersection    = 0x1000,
        Callable        = 0x2000,
        AllRayTracing   = 0x3F00,

        All             = 0x3FFF,
    };

    struct FShaderDesc
    {
        EShaderType ShaderType = EShaderType::None;
        std::string DebugName;
        std::string EntryName = "main";

        constexpr FShaderDesc& SetShaderType(EShaderType Value) { ShaderType = Value; return *this; }
        FShaderDesc& SetDebugName(const std::string& Value) { DebugName = Value; return *this; }
        FShaderDesc& SetEntryName(const std::string& Value) { EntryName = Value; return *this; }
    };

    class IShader : public IResource
    {
    public:
        [[nodiscard]] virtual const FShaderDesc& GetDesc() const = 0;
        virtual void GetByteCode(const void** ppByteCode, size_t* pSize) const = 0;
    };

    typedef RefCountPtr<IShader> ShaderHandle;

    class IShaderLibrary : public IResource
    {
    public:
        virtual void GetByteCode(const void** ppByteCode, size_t* pSize) const = 0;
        virtual ShaderHandle GetShader(const char* EntryName, EShaderType ShaderType) = 0;
    };

    typedef RefCountPtr<IShaderLibrary> ShaderLibraryHandle;
}
