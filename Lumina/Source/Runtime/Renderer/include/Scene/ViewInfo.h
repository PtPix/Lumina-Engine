/**
  * @file ViewInfo.h
  * @brief View rendering information with GPU data.
  *
  * Render-thread representation of a view, contains GPU buffers and computed data.
  */

#pragma once

#include "SharedTypes.h"
#include "RenderTypes.h"
#include <DirectXMath.h>

class Camera;

// View information for rendering
class FViewInfo
{
public:
    FViewInfo() = default;
    ~FViewInfo() = default;

    // Setup from Engine Camera
    void SetupFromCamera(Camera* Camera, uint32_t RenderWidth, uint32_t RenderHeight);

    // Matrices
    DirectX::XMFLOAT4X4 ViewMatrix;
    DirectX::XMFLOAT4X4 ProjectionMatrix;
    DirectX::XMFLOAT4X4 ViewProjectionMatrix;

    // Camera position
    DirectX::XMFLOAT4 CameraPosition;

    // Viewport
    uint32_t RenderTargetWidth = 1920;
    uint32_t RenderTargetHeight = 1080;
};