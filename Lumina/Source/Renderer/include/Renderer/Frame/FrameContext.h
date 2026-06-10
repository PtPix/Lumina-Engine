#pragma once
#include <cstdint>

#include "Renderer/D3D12Core/Core/FCommandContext.h"

class FFrameContext
{
private:
    uint64_t mFrameIndex = 0;
    uint32_t mBackBufferIndex = 0;

    FCommandContext* GraphicsContext = nullptr;
    FCommandContext* ComputeContext = nullptr;
    FCommandContext* CopyContext = nullptr;
};
