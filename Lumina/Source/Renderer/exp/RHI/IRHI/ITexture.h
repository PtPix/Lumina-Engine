#pragma once

#include <cstdint>
#include <string>

#include "RHITypes.h"
#include "Renderer/RHI/common/Resources.h"

namespace LuminaRHI
{
    enum class ETextureDimension : uint8_t
    {
        Unknown,
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        TextureCube,
        TextureCubeArray,
        Texture2DMS,
        Texture2DMSArray,
        Texture3D
    };

    struct FTextureDesc
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        uint32_t Depth = 1;
        uint32_t ArraySize = 1;
        uint32_t MipLevels = 1;
        uint32_t SampleCount = 1;
        uint32_t SampleQuality = 0;
        EFormat Format = EFormat::UNKNOWN;
        ETextureDimension Dimension = ETextureDimension::Texture2D;
        std::string DebugName;

        bool bIsShaderResource = true;
        bool bIsRenderTarget = false;
        bool bIsUAV = false;

        FColor ClearValue;
        bool bUseClearValue = false;

        constexpr FTextureDesc& SetWidth(uint32_t Value) { Width = Value; return *this; }
        constexpr FTextureDesc& SetHeight(uint32_t Value) { Height = Value; return *this; }
        constexpr FTextureDesc& SetDepth(uint32_t Value) { Depth = Value; return *this; }
        constexpr FTextureDesc& SetArraySize(uint32_t Value) { ArraySize = Value; return *this; }
        constexpr FTextureDesc& SetMipLevels(uint32_t Value) { MipLevels = Value; return *this; }
        constexpr FTextureDesc& SetSampleCount(uint32_t Value) { SampleCount = Value; return *this; }
        constexpr FTextureDesc& SetSampleQuality(uint32_t Value) { SampleQuality = Value; return *this; }
        constexpr FTextureDesc& SetFormat(EFormat Value) { Format = Value; return *this; }
        constexpr FTextureDesc& SetDimension(ETextureDimension Value) { Dimension = Value; return *this; }
        FTextureDesc& SetDebugName(const std::string& Value) { DebugName = Value; return *this; }
        constexpr FTextureDesc& SetIsRenderTarget(bool Value) { bIsRenderTarget = Value; return *this; }
        constexpr FTextureDesc& SetIsUAV(bool Value) { bIsUAV = Value; return *this; }
        constexpr FTextureDesc& SetClearValue(const FColor& Value) { ClearValue = Value; bUseClearValue = true; return *this; }
        constexpr FTextureDesc& SetUseClearValue(bool Value) { bUseClearValue = Value; return *this; }
    };

    class ITexture : public IResource
    {
    public:
        [[nodiscard]] virtual const FTextureDesc& GetDesc() const = 0;
    };

    typedef RefCountPtr<ITexture> TextureHandle;
}
