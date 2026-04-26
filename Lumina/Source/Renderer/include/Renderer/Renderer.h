#pragma once

#include <windows.h>
#include <cstdint>

class FCommandContext;

class Renderer
{
public:
    static bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    static void Shutdown();

    static FCommandContext* BeginFrame();

    static void EndFrame(FCommandContext* pContext);

};