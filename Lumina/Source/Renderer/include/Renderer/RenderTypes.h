#pragma once

#include <DirectXMath.h>
#include <vector>
#include <cstdint>

struct FObjectData
{
    DirectX::XMMATRIX WorldMatrix;
    DirectX::XMFLOAT4 BaseColor;

    uint32_t AlbedoTexIndex;
    uint32_t NormalTexIndex;
    uint32_t RMATexIndex;
    uint32_t Padding;
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