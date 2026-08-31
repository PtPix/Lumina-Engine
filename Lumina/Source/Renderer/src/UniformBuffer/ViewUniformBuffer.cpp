#include "Renderer/UniformBuffer/ViewUniformBuffer.h"
#include "Renderer/Core/RendererCore.h"

using namespace DirectX;

FViewUniformData FViewUniformBuilder::Build(const DirectX::XMMATRIX &ViewMatrix,
    const DirectX::XMMATRIX &ProjectionMatrix, const DirectX::XMFLOAT3 &ViewOrigin, uint32_t ViewportWidth,
    uint32_t ViewportHeight)
{
    FViewUniformData Data = {};

    XMMATRIX ViewProj = XMMatrixMultiply(ViewMatrix, ProjectionMatrix);
    XMMATRIX InvView = XMMatrixInverse(nullptr, ViewMatrix);
    XMMATRIX InvProj = XMMatrixInverse(nullptr, ProjectionMatrix);
    XMMATRIX InvViewProj = XMMatrixInverse(nullptr, ViewProj);

    Data.MatView = XMMatrixTranspose(ViewMatrix);
    Data.MatProjection = XMMatrixTranspose(ProjectionMatrix);
    Data.MatViewProjection = XMMatrixTranspose(ViewProj);
    Data.MatInvView = XMMatrixTranspose(InvView);
    Data.MatInvProjection = XMMatrixTranspose(InvProj);
    Data.MatInvViewProjection = XMMatrixTranspose(InvViewProj);

    Data.ViewOrigin = ViewOrigin;

    XMVECTOR Forward = XMVector3Normalize(ViewMatrix.r[2]);
    XMStoreFloat3(&Data.ViewDirection, Forward);

    Data.ViewportWidth = ViewportWidth;
    Data.ViewportHeight = ViewportHeight;
    Data.FrameIndex = FRendererCore::GetFrameIndex();

    Data.SunDirection = { 0.577f, -0.577f, 0.577f };
    Data.SunIntensity = 1.0f;
    Data.SunColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    return Data;
}
