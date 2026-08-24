#include "Renderer/Scene/FViewInfo.h"

using namespace DirectX;

void FViewInfo::SetupFromMatrices(const DirectX::XMMATRIX &View, const DirectX::XMMATRIX &Projection,
    const DirectX::XMFLOAT3 &CameraPosition, uint32_t ViewportWidth, uint32_t ViewportHeight, uint32_t FrameIndex)
{
    ShaderInterop::SetTransposedMatrix(mViewUniform.ViewMatrix, View);
    ShaderInterop::SetTransposedMatrix(mViewUniform.ProjectionMatrix, Projection);

    const XMMATRIX VP = XMMatrixMultiply(View, Projection);
    ShaderInterop::SetTransposedMatrix(mViewUniform.ViewProjectionMatrix, VP);

    XMVECTOR Det;
    const XMMATRIX InvView = XMMatrixInverse(&Det, View);
    const XMMATRIX InvProj = XMMatrixInverse(&Det, Projection);
    const XMMATRIX InvVP = XMMatrixInverse(&Det, VP);

    ShaderInterop::SetTransposedMatrix(mViewUniform.InvViewMatrix, InvView);
    ShaderInterop::SetTransposedMatrix(mViewUniform.InvProjectionMatrix, InvProj);
    ShaderInterop::SetTransposedMatrix(mViewUniform.InvViewProjectionMatrix, InvVP);

    ShaderInterop::SetTransposedMatrix(mViewUniform.PrevViewProjectionMatrix, VP);

    mViewUniform.CameraPosition = CameraPosition;

    XMFLOAT4X4 ViewF = {};
    XMStoreFloat4x4(&ViewF, View);
    mViewUniform.CameraDirection = XMFLOAT3(-ViewF._13, -ViewF._23, -ViewF._33);

    mViewUniform.NearPlane = 0.1f;
    mViewUniform.FarPlane = 1000.0f;

    mViewUniform.ViewportSize = XMFLOAT2(static_cast<float>(ViewportWidth), static_cast<float>(ViewportHeight));
    mViewUniform.ViewportSizeRcp = XMFLOAT2(1.0f / ViewportWidth, 1.0f / ViewportHeight);

    mViewUniform.Jitter = XMFLOAT2(0.0f, 0.0f);
    mViewUniform.FrameIndex = FrameIndex;
    mViewUniform._ViewPad1 = 0;
}

DirectX::XMMATRIX FViewInfo::MakeProjectionMatrixReverseZ(float FovYRadians, float AspectRatio, float NearZ, float FarZ)
{
    // Projection: Z' = (f / (f - n)) * Z - (f * n / (f - n))
    // Reverse Projection: Z'' = 1 - Z' = Z'' = (n / (n - f)) * Z + (f * n / (n - f))
    const float Height = 1.0f / tanf(FovYRadians * 0.5f);
    const float Width = Height / AspectRatio;

    // f -> Inf, (n / (n - f)) -> 0, (f * n / (n - f)) -> n
    XMMATRIX M = XMMatrixIdentity();
    M.r[0].m128_f32[0] = Width;
    M.r[1].m128_f32[1] = Height;
    M.r[2].m128_f32[2] = 0.0f;
    M.r[2].m128_f32[3] = 1.0f;
    M.r[3].m128_f32[2] = NearZ;
    M.r[3].m128_f32[3] = 0.0f;

    return M;
}
