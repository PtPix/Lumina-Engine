#include "Renderer/SceneRendering/ViewInfo.h"
#include "Renderer/Scene/SceneView.h"
#include "Renderer/Core/RendererCore.h"

using namespace DirectX;

void FViewInfo::Initialize()
{
    mViewUniformBuffer.Create(L"ViewUniformBuffer");
}

void FViewInfo::SetupFromSceneView(const FSceneView &SceneView, uint32_t FrameIndex)
{
    mViewMatrix = SceneView.GetViewMatrix();
    mProjectionMatrix = SceneView.GetProjectionMatrix();
    mViewLocation = SceneView.GetViewLocation();
    mViewportWidth = SceneView.GetViewportWidth();
    mViewportHeight = SceneView.GetViewportHeight();

    mViewUniformData = FViewUniformBuilder::Build(
        mViewMatrix, mProjectionMatrix, mViewLocation, mViewportWidth, mViewportHeight);

    mViewUniformBuffer.Update(mViewUniformData, FrameIndex);
}

D3D12_GPU_VIRTUAL_ADDRESS FViewInfo::GetViewUniformGPUAddress(uint32_t FrameIndex) const
{
    return mViewUniformBuffer.GetGPUAddress(FrameIndex);
}