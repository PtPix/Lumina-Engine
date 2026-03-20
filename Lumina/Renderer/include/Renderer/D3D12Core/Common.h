#pragma once
#include <d3d12.h>
#include <cstdio>

template<class T>
T AlignOffset(const T& uOffset, const T& uAlign) { return ((uOffset + (uAlign - 1)) & ~(uAlign - 1)); }

template<class... Args>
void SetName(ID3D12Object* pObject, const char* Format, Args&&... args)
{
    char BufferName[240];
    sprintf_s(BufferName, Format, args...);

    wchar_t WBufferName[240];
    size_t ConvertedChars = 0;
    mbstowcs_s(&ConvertedChars, WBufferName, BufferName, sizeof(WBufferName) / sizeof(wchar_t));

    pObject->SetName(WBufferName);
}

static constexpr uint32_t NUM_SWAPCHAIN_BACKBUFFER = 2;
