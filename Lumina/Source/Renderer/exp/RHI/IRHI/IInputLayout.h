#pragma once

#include <cstdint>
#include <string>

#include "RHITypes.h"
#include "Renderer/RHI/common/Resources.h"

namespace LuminaRHI
{
    struct FVertexAttributeDesc
    {
        std::string Name;
        EFormat Format = EFormat::UNKNOWN;
        uint32_t ArraySize = 1;
        uint32_t BufferIndex = 0;
        uint32_t Offset = 0;
        uint32_t ElementStride = 0;
        bool IsInstanced = false;

        FVertexAttributeDesc& SetName(const std::string& Value) { Name = Value; return *this; }
        constexpr FVertexAttributeDesc& SetFormat(EFormat Value) { Format = Value; return *this; }
        constexpr FVertexAttributeDesc& SetArraySize(uint32_t Value) { ArraySize = Value; return *this; }
        constexpr FVertexAttributeDesc& SetBufferIndex(uint32_t Value) { BufferIndex = Value; return *this; }
        constexpr FVertexAttributeDesc& SetOffset(uint32_t Value) { Offset = Value; return *this; }
        constexpr FVertexAttributeDesc& SetElementStride(uint32_t Value) { ElementStride = Value; return *this; }
        constexpr FVertexAttributeDesc& SetIsInstanced(bool Value) { IsInstanced = Value; return *this; }
    };

    class IInputLayout : public IResource
    {
    public:
        [[nodiscard]] virtual uint32_t GetNumAttributes() const = 0;
        [[nodiscard]] virtual const FVertexAttributeDesc& GetAttributeDesc(uint32_t Index) const = 0;
    };

    typedef RefCountPtr<IInputLayout> InputLayoutHandle;
}
