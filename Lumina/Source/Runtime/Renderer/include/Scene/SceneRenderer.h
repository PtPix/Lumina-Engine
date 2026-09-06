/**
  * @file SceneRenderer.h
  * @brief Scene renderer coordinating all render passes.
  *
  * Takes FSceneView (input) and coordinates rendering through all passes.
  * Builds draw commands, manages view info, and orchestrates the render graph.
  */

#pragma once

#include "ViewInfo.h"
#include "MeshDrawCommand.h"
#include "RenderTypes.h"
#include <vector>

class FRenderGraph;
class FSceneView;

class FSceneRenderer
{
public:
    FSceneRenderer() = default;
    ~FSceneRenderer() = default;

    void Initialize();
    void Shutdown();

    void Render(FRenderGraph& RenderGraph, const FSceneView& View);

    [[nodiscard]] const FViewInfo& GetViewInfo() const { return mViewInfo; }
    [[nodiscard]] FViewInfo& GetViewInfo() { return mViewInfo; }

    [[nodiscard]] const std::vector<FMeshDrawCommand>& GetDrawCommands() const { return mDrawCommands; }

private:
    void GatherDrawCommands(const FSceneView& View);
    void SortDrawCommands();

    void RenderBasePass(FRenderGraph& RG);
    void RenderLighting(FRenderGraph& RG);
    void RenderPostProcess(FRenderGraph& RG);

private:
    FViewInfo mViewInfo;
    std::vector<FMeshDrawCommand> mDrawCommands;

    bool mbInitialized = false;
};