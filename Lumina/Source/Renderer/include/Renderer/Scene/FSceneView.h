#pragma once

#include <DirectXMath.h>

#include "Renderer/RenderTypes.h"
#include "Renderer/Resources/FMaterial.h"

// Per Instance Data
struct alignas(16) FInstanceData
{
    DirectX::XMMATRIX WorldMatrix;
    uint32_t MaterialIndex;
    uint32_t Pad[3];
};

struct FDrawCommand
{
    class FMesh* pMesh;
    uint32_t InstanceIndex;
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