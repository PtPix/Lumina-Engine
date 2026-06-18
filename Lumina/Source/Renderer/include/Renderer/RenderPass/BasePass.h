#pragma once

#include "Renderer/RenderGraph/RenderGraph.h"

struct FSceneView;

/**
 * @file BasePass.h
 * @brief Forward base pass as a composable render-graph feature.
 *
 * Mirrors the UE-style "AddXxxPass(GraphBuilder, View, Inputs, &Outputs)"
 * shape: the caller supplies the color target to render into; the pass
 * creates its own depth target and returns the handle via Outputs.
 */

struct FBasePassInputs
{
    FRGTextureHandle SceneColor;     // 写入目标(由调用方提供,通常是 EditorViewport.SceneColor)
    const FSceneView* View = nullptr;

    uint32_t ViewportWidth  = 0;     // 渲染分辨率(0 => 用 backend 当前尺寸)
    uint32_t ViewportHeight = 0;
};

struct FBasePassOutputs
{
    FRGTextureHandle SceneDepth;     // pass 内部创建,回传调用方供后续 pass 使用
};

// 声明 BasePass 到 render graph。AddPass 粒度保持显式。
void AddBasePass(FRenderGraph& Graph,
                 const FBasePassInputs& Inputs,
                 FBasePassOutputs& Outputs);