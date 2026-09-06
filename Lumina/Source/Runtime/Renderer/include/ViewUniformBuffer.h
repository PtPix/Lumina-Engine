/**
  * @file ViewUniformBuffer.h
  * @brief View-level uniform buffer (camera, global lighting).
  *
  * Contains per-view data: matrices, camera position, global lighting.
  * Updated once per view per frame.
  */
#pragma once

#include "UniformBuffer.h"
#include "SharedTypes.h"
#include <DirectXMath.h>

struct alignas(256) FViewUniformData
{
    DirectX::XMMATRIX MatView;
    DirectX::XMMATRIX MatProjection;
    DirectX::XMMATRIX MatViewProjection;
    DirectX::XMMATRIX MatInvView;
    DirectX::XMMATRIX MatInvProjection;
    DirectX::XMMATRIX MatInvViewProjection;

    DirectX::XMFLOAT3 ViewOrigin;
    float Padding0;

    DirectX::XMFLOAT3 ViewDirection;
    float Padding1;

    DirectX::XMFLOAT3 SunDirection;
    float SunIntensity;

    DirectX::XMFLOAT4 SunColor;

    uint32_t ViewportWidth;
    uint32_t ViewportHeight;
    uint32_t FrameIndex;
    uint32_t Padding2;

    // Add more as needed (ambient, fog, etc.)
};

static_assert(sizeof(FViewUniformData) % 256 == 0, "ViewUniformData must be 256-byte aligned for constant buffer");

using FViewUniformBuffer = TUniformBuffer<FViewUniformData>;

class FViewUniformBuilder
{
public:
    static FViewUniformData Build(const DirectX::XMMATRIX& ViewMatrix, const DirectX::XMMATRIX& ProjectionMatrix,
        const DirectX::XMFLOAT3& ViewOrigin, uint32_t ViewportWidth, uint32_t ViewportHeight);
};