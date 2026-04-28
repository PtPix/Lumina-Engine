#pragma once

#include "Renderer/RenderTypes.h"
#include "Renderer/D3D12Core/Resource/Buffer.h"

class FMesh;

struct FSceneObject
{
    FMesh* pMesh = nullptr;

    FBuffer ObjectDataBuffer;
    uint32_t BindlessIndex = 0xFFFFFFFF;
};