#include "Renderer/Scene/SceneView.h"

using namespace DirectX;

FSceneView::FSceneView(const FSceneViewInitOptions &InitOptions)
    : mViewMatrix(InitOptions.ViewMatrix)
    , mProjectionMatrix(InitOptions.ProjectionMatrix)
    , mViewLocation(InitOptions.ViewLocation)
    , mViewDirection(InitOptions.ViewDirection)
    , mViewportWidth(InitOptions.ViewportWidth)
    , mViewportHeight(InitOptions.ViewportHeight)
    , mNearClipPlane(InitOptions.NearClipPlane)
    , mFarClipPlane(InitOptions.FarClipPlane) {}

DirectX::XMMATRIX FSceneView::GetViewProjectionMatrix() const
{
    return XMMatrixMultiply(mViewMatrix, mProjectionMatrix);
}

DirectX::XMMATRIX FSceneView::GetInvViewMatrix() const
{
    return XMMatrixInverse(nullptr, mViewMatrix);
}

DirectX::XMMATRIX FSceneView::GetInvProjectionMatrix() const
{
    return XMMatrixInverse(nullptr, mProjectionMatrix);
}


