#include "Renderer/SceneRendering/SceneRenderer.h"
#include "Renderer/Scene/SceneView.h"
#include "Renderer/RenderGraph/RenderGraph.h"
#include "Renderer/Core/RendererCore.h"

#include <algorithm>

void FSceneRenderer::Initialize()
{
    if (mbInitialized) return;

    mViewInfo.Initialize();

    mbInitialized = true;
}

void FSceneRenderer::Shutdown()
{
    if (!mbInitialized) return;

    mDrawCommands.clear();
    mbInitialized = false;
}

void FSceneRenderer::Render(FRenderGraph &RenderGraph, const FSceneView &View)
{
    uint32_t FrameIndex = FRendererCore::GetFrameIndex();

    mViewInfo.SetupFromSceneView(View, FrameIndex);

    GatherDrawCommands(View);

    SortDrawCommands();

    RenderBasePass(RenderGraph);
    RenderLighting(RenderGraph);
    RenderPostProcess(RenderGraph);
}

void FSceneRenderer::GatherDrawCommands(const FSceneView &View)
{
    mDrawCommands.clear();

    // TODO : Query scene primitives and build draw commands
}

void FSceneRenderer::SortDrawCommands()
{
    std::sort(mDrawCommands.begin(), mDrawCommands.end(),
        [](const FMeshDrawCommand& A, const FMeshDrawCommand& B) {return A.SortKey < B.SortKey;});
}

void FSceneRenderer::RenderBasePass(FRenderGraph &RG)
{
    // TODO: Future pass
}

void FSceneRenderer::RenderLighting(FRenderGraph &RG)
{
    // TODO: Future pass
}

void FSceneRenderer::RenderPostProcess(FRenderGraph &RG)
{
    // TODO: Future pass
}
