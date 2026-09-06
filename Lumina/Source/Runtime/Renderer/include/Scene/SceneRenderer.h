#pragma once

#include "RenderGraph/RenderGraph.h"
#include "Scene/ViewInfo.h"
#include "Scene/MeshDrawCommand.h"
#include <vector>

class FScene;
class Camera;

/**
 * @brief High-level scene renderer
 *
 * Orchestrates rendering of a scene from Engine layer.
 * Integrates Camera, Scene, and renders through RenderGraph.
 */
class FSceneRenderer
{
public:
    FSceneRenderer() = default;
    ~FSceneRenderer() = default;

    /**
     * @brief Render a scene from a camera viewpoint
     *
     * @param Scene The scene containing GameObjects and Materials
     * @param Camera The camera defining the viewpoint
     * @param RenderGraph The render graph to add passes to
     */
    void RenderScene(FScene* Scene, Camera* Camera, FRenderGraph& RenderGraph);

private:
    void SetupViewFromCamera(Camera* Camera);
    void GatherDrawCommands(FScene* Scene);
    void BuildRenderPasses(FRenderGraph& RenderGraph);

private:
    FViewInfo mViewInfo;
    std::vector<FMeshDrawCommand> mDrawCommands;
};
