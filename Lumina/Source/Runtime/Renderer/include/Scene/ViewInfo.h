/**
  * @file ViewInfo.h
  * @brief View rendering information with GPU data.
  *
  * Render-thread representation of a view, contains GPU buffers and computed data.
  * Created from FSceneView (game thread input).
  */

#pragma once

#include "SharedTypes.h"
#include "D3D12Buffer.h"
#include "RenderTypes.h"
#include "ViewUniformBuffer.h"
#include <DirectXMath.h>

class FSceneView;

// Render Thread view information
class FViewInfo
{
public:
    FViewInfo() = default;
    ~FViewInfo() = default;

    void Initialize();
    void SetupFromSceneView(const FSceneView& SceneView, uint32_t FrameIndex);

    // Uniform buffer access
    [[nodiscard]] const FViewUniformData& GetViewUniform() const { return mViewUniformData; }
    [[nodiscard]] FViewUniformData& GetViewUniform() { return mViewUniformData; }

    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetViewUniformGPUAddress(uint32_t FrameIndex) const;

    // Matrices
    [[nodiscard]] const DirectX::XMMATRIX& GetViewMatrix() const { return mViewMatrix; }
    [[nodiscard]] const DirectX::XMMATRIX& GetProjectionMatrix() const { return mProjectionMatrix; }
    [[nodiscard]] DirectX::XMMATRIX GetViewProjectionMatrix() const { return XMMatrixMultiply(mViewMatrix, mProjectionMatrix); }

    // Camera Properties
    [[nodiscard]] const DirectX::XMFLOAT3& GetViewLocation() const { return mViewLocation; }
    [[nodiscard]] uint32_t GetViewportWidth() const { return mViewportWidth; }
    [[nodiscard]] uint32_t GetViewportHeight() const { return mViewportHeight; }

private:
    // CPU-side data
    DirectX::XMMATRIX mViewMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX mProjectionMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT3 mViewLocation = { 0.0f, 0.0f, 0.0f };

    uint32_t mViewportWidth = 1920;
    uint32_t mViewportHeight = 1080;

    // GPU-side data
    FViewUniformData mViewUniformData;
    FViewUniformBuffer mViewUniformBuffer;
};