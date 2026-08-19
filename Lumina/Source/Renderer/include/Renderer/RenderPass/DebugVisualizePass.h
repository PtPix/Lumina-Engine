#pragma once

#include "Renderer/RenderGraph/RenderGraph.h"

struct FDebugVisualizeInputs
{
    FRGTextureHandle SceneDepth;
    uint32_t Width = 0;
    uint32_t Height = 0;
};

struct FDebugVisualizeOutputs
{
    FRGTextureHandle VisualizedDepth;
};

void AddDebugVisualizeDepthPass(FRenderGraph& Graph, const FDebugVisualizeInputs& Inputs, FDebugVisualizeOutputs& Outputs);