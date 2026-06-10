#pragma once
#include <cstdint>

#include "RHITypes.h"
#include "Renderer/RHI/common/Resources.h"

namespace LuminaRHI
{
    enum class ESamplerAddressMode : uint8_t
    {
        Clamp,
        Wrap,
        Border,
        Mirror,
        MirrorOnce,
    };

    struct FSamplerDesc
    {
        FColor BorderColor = 1.0f;
        float MaxAnisotropy = 1.0f;
        float MipBias = 0.0f;

        bool MinFilter = true;
        bool MagFilter = true;
        bool MipFilter = true;
        ESamplerAddressMode AddressU = ESamplerAddressMode::Clamp;
        ESamplerAddressMode AddressV = ESamplerAddressMode::Clamp;
        ESamplerAddressMode AddressW = ESamplerAddressMode::Clamp;

        FSamplerDesc& SetBorderColor(const FColor& Value) { BorderColor = Value; return *this; }
        FSamplerDesc& SetMaxAnisotropy(float Value) { MaxAnisotropy = Value; return *this; }
        FSamplerDesc& SetMipBias(float Value) { MipBias = Value; return *this; }
        FSamplerDesc& SetMinFilter(bool Value) { MinFilter = Value; return *this; }
        FSamplerDesc& SetMagFilter(bool Value) { MagFilter = Value; return *this; }
        FSamplerDesc& SetMipFilter(bool Value) { MipFilter = Value; return *this; }
        FSamplerDesc& SetAddressU(ESamplerAddressMode Value) { AddressU = Value; return *this; }
        FSamplerDesc& SetAddressV(ESamplerAddressMode Value) { AddressV = Value; return *this;}
        FSamplerDesc& SetAddressW(ESamplerAddressMode Value) { AddressW = Value; return *this;}
    };

    class ISampler : public IResource
    {
    public:
        [[nodiscard]] virtual const FSamplerDesc& GetDesc() const = 0;
    };

    typedef RefCountPtr<ISampler> SamplerHandle;
}
