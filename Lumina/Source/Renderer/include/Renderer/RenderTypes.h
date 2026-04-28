#pragma once

#include <DirectXMath.h>
#include <vector>
#include <cstdint>

struct FObjectData
{
    DirectX::XMMATRIX WorldMatrix;
    DirectX::XMFLOAT4 BaseColor;
    float Metallic;
    float Roughness;
    DirectX::XMFLOAT2 Padding;
};

struct FGlobalPassData
{
    DirectX::XMMATRIX ViewProjectionMatrix;
    DirectX::XMFLOAT3 CameraPosition;
    float Padding1;

    DirectX::XMFLOAT3 SunDirection;
    float SunIntensity;

    DirectX::XMFLOAT4 SunColor;
};