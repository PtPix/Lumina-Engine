#pragma once
#include <cstdint>
#include <string>

#include "RHITypes.h"
#include "Renderer/RHI/common/Resources.h"

namespace LuminaRHI
{
    struct FBufferDesc
    {
        uint64_t ByteSize = 0;
        uint32_t StructStride = 0;
        std::string DebugName;
        EFormat Format = EFormat::UNKNOWN;
        bool bIsVertexBuffer = false;
        bool bIsIndexBuffer = false;
        bool bIsConstantBuffer = false;

        EResourceStates InitialState = EResourceStates::Common;
        ECpuAccessMode CpuAccessMode = ECpuAccessMode::None;

        constexpr FBufferDesc& SetByteSize(uint64_t Value) { ByteSize = Value; return *this; }
        constexpr FBufferDesc& SetStructStride(uint64_t Value) { StructStride = Value; return *this; }
        FBufferDesc& SetDebugName(const std::string& Value) { DebugName = Value; return *this; }
        constexpr FBufferDesc& SetFormat(EFormat Value) { Format = Value; return *this; }
        constexpr FBufferDesc& SetIsVertexBuffer(bool Value) { bIsVertexBuffer = Value; return *this; }
        constexpr FBufferDesc& SetIsIndexBuffer(bool Value) { bIsIndexBuffer = Value; return *this; }
        constexpr FBufferDesc& SetConstantBuffer(bool Value) { bIsConstantBuffer = Value; return *this; }
        constexpr FBufferDesc& SetInitialState(EResourceStates Value) { InitialState = Value; return *this; }
        constexpr FBufferDesc& SetCpuAccessMode(ECpuAccessMode Value) { CpuAccessMode = Value; return *this; }
    };

    class IBuffer : public IResource
    {
    public:
        [[nodiscard]] virtual const FBufferDesc& GetDesc() const = 0;
        [[nodiscard]] virtual GpuVirtualAddress GetGpuVirtualAddress() const = 0;
    };

    typedef RefCountPtr<IBuffer> BufferHandle;
}
