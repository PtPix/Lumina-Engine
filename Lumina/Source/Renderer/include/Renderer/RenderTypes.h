#pragma once

#include <DirectXMath.h>

struct FGlobalPassData
{
    DirectX::XMMATRIX ViewProjectionMatrix;
    DirectX::XMFLOAT3 CameraPosition;
    float Padding1;

    DirectX::XMFLOAT3 SunDirection;
    float SunIntensity;

    DirectX::XMFLOAT4 SunColor;
};