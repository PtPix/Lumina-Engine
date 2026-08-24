#pragma once

#include "Renderer/ShaderInterop/SharedTypes.h"
#include "Renderer/RenderTypes.h"

#include <DirectXMath.h>
#include <vector>

struct FDrawCommand
{
    class FMesh* pMesh;
    uint32_t InstanceIndex;
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

struct FSceneView
{
    // Light, Camera
    FGlobalPassData GlobalPassData;

    // Copy to GPU
    std::vector<FInstanceData> InstanceData;
    std::vector<FPBRMaterialData> MaterialData;

    // DrawCommand
    std::vector<FDrawCommand> DrawCommands;

    void Clear()
    {
        InstanceData.clear();
        MaterialData.clear();
        DrawCommands.clear();
    }
};