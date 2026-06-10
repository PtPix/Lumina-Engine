#pragma once

#include <cstdint>
#include <string>

#include "Renderer/RHI/common/Resources.h"

namespace LuminaRHI
{
    enum class EHeapType : uint8_t
    {
        DeviceLocal,
        Upload,
        Readback
    };

    struct FHeapDesc
    {
        uint64_t Capacity = 0;
        EHeapType Type;
        std::string DebugName;

        constexpr FHeapDesc& SetCapacity(uint64_t Value) { Capacity = Value; return *this; }
        constexpr FHeapDesc& SetType(EHeapType Value) { Type = Value; return *this; }
        FHeapDesc& SetDebugName(const std::string Value) { DebugName = Value; return *this; }
    };

    class IHeap : public IResource
    {
    public:
        virtual const FHeapDesc& GetDesc() = 0;
    };

    typedef RefCountPtr<IHeap> HeapHandle;
}
