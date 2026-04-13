#pragma once

#include <d3d12.h>
#include <cstdio>
#include <string>

#include "StringUtils/StringConv.h"

// T must be integer type
template<class T>
T AlignOffset(const T& uOffset, const T& uAlign)
{
    static_assert(std::is_integral_v<T>, "Alignment must be an integral type");
    return ((uOffset + (uAlign - 1)) & ~(uAlign - 1));
}

// Dynamic Name Buffer with UTF8
inline void SetName(ID3D12Object* pObject, const char* Format, ...)
{
    if (!pObject || !Format)
    {
        return;
    }

    // Get Buffer Size
    va_list Args;
    va_start(Args, Format);
    uint32_t Size = std::vsnprintf(nullptr, 0, Format, Args);
    va_end(Args);
    if (Size <= 0)
    {
        return;
    }

    // Allocate Buffer
    std::string Utf8Buffer(Size, '\0');
    va_start(Args, Format);
    std::vsnprintf(Utf8Buffer.data(), Size + 1, Format, Args);
    va_end(Args);

    // Calculate UTF16 size
    int WSize = MultiByteToWideChar(CP_UTF8, 0, Utf8Buffer.c_str(), -1, nullptr, 0);
    if (WSize <= 0) return;
    std::wstring Utf16Buffer = StringUtils::UTF8ToWide(Utf8Buffer);

    // Set Name
    pObject->SetName(Utf16Buffer.c_str());
}

inline constexpr uint32_t NUM_SWAPCHAIN_BACKBUFFER = 3;
