#pragma once

#include "Renderer/ShaderInterop/SharedTypes.h"

#include <DirectXMath.h>
#include <vector>

class FViewInfo
{
public:
    FViewInfo() = default;

    void SetupFromMatrices(const DirectX::XMMATRIX& View, const DirectX::XMMATRIX& Projection,
        const DirectX::XMFLOAT3& CameraPosition, uint32_t ViewportWidth, uint32_t ViewportHeight, uint32_t FrameIndex);

    static DirectX::XMMATRIX MakeProjectionMatrixReverseZ(
        float FovYRadians, float AspectRatio, float NearZ, float FarZ);

    [[nodiscard]] const FViewUniform& GetViewUniform() const { return mViewUniform; }
    [[nodiscard]] FViewUniform& GetViewUniform() { return mViewUniform; }

private:
    FViewUniform mViewUniform = {};
};