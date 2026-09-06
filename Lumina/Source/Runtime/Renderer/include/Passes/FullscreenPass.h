/**
  * @file FullscreenPass.h
  * @brief Base class for fullscreen passes (post-process, debug, etc.).
  */

#pragma once

#include "RenderPass.h"

class FFullscreenPassBase : public FGraphicsPassBase
{
public:
    explicit FFullscreenPassBase(const char* PassName) : FGraphicsPassBase(PassName) {}

    ~FFullscreenPassBase() override = default;

protected:
    void DrawFullscreenTriangle(FD3D12CommandContext* CommandContext);
};