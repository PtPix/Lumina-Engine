#pragma once

#include <cmath>
#include <cstdint>

namespace LuminaRHI
{
    typedef uint64_t GpuVirtualAddress;

    struct FColor
    {
        float r, g, b, a;

        FColor() : r(0.f), g(0.f), b(0.f), a(0.f) { }
        FColor(float c) : r(c), g(c), b(c), a(c) { }
        FColor(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) { }

        bool operator ==(const FColor& _b) const { return r == _b.r && g == _b.g && b == _b.b && a == _b.a; }
        bool operator !=(const FColor& _b) const { return !(*this == _b); }
    };

    struct FViewport
    {
        float minX, maxX;
        float minY, maxY;
        float minZ, maxZ;

        FViewport() : minX(0.f), maxX(0.f), minY(0.f), maxY(0.f), minZ(0.f), maxZ(1.f) { }

        FViewport(float width, float height) : minX(0.f), maxX(width), minY(0.f), maxY(height), minZ(0.f), maxZ(1.f) { }

        FViewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ)
            : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY), minZ(_minZ), maxZ(_maxZ)
        { }

        bool operator ==(const FViewport& b) const
        {
            return minX == b.minX
                && minY == b.minY
                && minZ == b.minZ
                && maxX == b.maxX
                && maxY == b.maxY
                && maxZ == b.maxZ;
        }
        bool operator !=(const FViewport& b) const { return !(*this == b); }

        [[nodiscard]] float width() const { return maxX - minX; }
        [[nodiscard]] float height() const { return maxY - minY; }
    };

    struct FRect
    {
        int minX, maxX;
        int minY, maxY;

        FRect() : minX(0), maxX(0), minY(0), maxY(0) { }
        FRect(int width, int height) : minX(0), maxX(width), minY(0), maxY(height) { }
        FRect(int _minX, int _maxX, int _minY, int _maxY) : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY) { }
        explicit FRect(const FViewport& viewport)
            : minX(int(floorf(viewport.minX)))
            , maxX(int(ceilf(viewport.maxX)))
            , minY(int(floorf(viewport.minY)))
            , maxY(int(ceilf(viewport.maxY)))
        {
        }

        bool operator ==(const FRect& b) const {
            return minX == b.minX && minY == b.minY && maxX == b.maxX && maxY == b.maxY;
        }
        bool operator !=(const FRect& b) const { return !(*this == b); }

        [[nodiscard]] int width() const { return maxX - minX; }
        [[nodiscard]] int height() const { return maxY - minY; }
    };

    enum class EFormat : uint8_t
    {
        UNKNOWN,

        R8_UINT,
        R8_SINT,
        R8_UNORM,
        R8_SNORM,
        RG8_UINT,
        RG8_SINT,
        RG8_UNORM,
        RG8_SNORM,
        R16_UINT,
        R16_SINT,
        R16_UNORM,
        R16_SNORM,
        R16_FLOAT,
        BGRA4_UNORM,
        B5G6R5_UNORM,
        B5G5R5A1_UNORM,
        RGBA8_UINT,
        RGBA8_SINT,
        RGBA8_UNORM,
        RGBA8_SNORM,
        BGRA8_UNORM,
        BGRX8_UNORM,
        SRGBA8_UNORM,
        SBGRA8_UNORM,
        SBGRX8_UNORM,
        R10G10B10A2_UNORM,
        R11G11B10_FLOAT,
        RG16_UINT,
        RG16_SINT,
        RG16_UNORM,
        RG16_SNORM,
        RG16_FLOAT,
        R32_UINT,
        R32_SINT,
        R32_FLOAT,
        RGBA16_UINT,
        RGBA16_SINT,
        RGBA16_FLOAT,
        RGBA16_UNORM,
        RGBA16_SNORM,
        RG32_UINT,
        RG32_SINT,
        RG32_FLOAT,
        RGB32_UINT,
        RGB32_SINT,
        RGB32_FLOAT,
        RGBA32_UINT,
        RGBA32_SINT,
        RGBA32_FLOAT,

        D16,
        D24S8,
        X24G8_UINT,
        D32,
        D32S8,
        X32G8_UINT,

        BC1_UNORM,
        BC1_UNORM_SRGB,
        BC2_UNORM,
        BC2_UNORM_SRGB,
        BC3_UNORM,
        BC3_UNORM_SRGB,
        BC4_UNORM,
        BC4_SNORM,
        BC5_UNORM,
        BC5_SNORM,
        BC6H_UFLOAT,
        BC6H_SFLOAT,
        BC7_UNORM,
        BC7_UNORM_SRGB,

        COUNT,
    };

    enum class EResourceStates : uint32_t
    {
        Unknown                     = 0,
        Common                      = 0x00000001,
        ConstantBuffer              = 0x00000002,
        VertexBuffer                = 0x00000004,
        IndexBuffer                 = 0x00000008,
        IndirectArgument            = 0x00000010,
        ShaderResource              = 0x00000020,
        UnorderedAccess             = 0x00000040,
        RenderTarget                = 0x00000080,
        DepthWrite                  = 0x00000100,
        DepthRead                   = 0x00000200,
        StreamOut                   = 0x00000400,
        CopyDest                    = 0x00000800,
        CopySource                  = 0x00001000,
        ResolveDest                 = 0x00002000,
        ResolveSource               = 0x00004000,
        Present                     = 0x00008000,
        AccelStructRead             = 0x00010000,
        AccelStructWrite            = 0x00020000,
        AccelStructBuildInput       = 0x00040000,
        AccelStructBuildBlas        = 0x00080000,
        ShadingRateSurface          = 0x00100000,
        OpacityMicromapWrite        = 0x00200000,
        OpacityMicromapBuildInput   = 0x00400000,
        ConvertCoopVecMatrixInput   = 0x00800000,
        ConvertCoopVecMatrixOutput  = 0x01000000,
    };

    enum class ECpuAccessMode : uint8_t
    {
        None,
        Read,
        Write
    };
}
