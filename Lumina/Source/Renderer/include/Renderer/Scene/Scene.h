#pragma once

#include "Renderer/RenderTypes.h"
#include "Renderer/D3D12Core/Resource/FBuffer.h"

class FMesh;

struct FSceneObject
{
    FMesh* pMesh = nullptr;
    uint32_t InstanceIndex;
};