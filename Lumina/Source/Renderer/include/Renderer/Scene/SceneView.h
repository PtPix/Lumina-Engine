#pragma once

#include <DirectXMath.h>

struct FSceneView
{
    DirectX::XMMATRIX ViewMatrix;
    DirectX::XMMATRIX ProjectionMatrix;
    DirectX::XMMATRIX ViewProjectionMatrix;
    DirectX::XMMATRIX InverseViewProjectionMatrix;

    uint32_t ViewportWidth;
    uint32_t ViewportHeight;
    DirectX::XMFLOAT3 CameraPosition;
};