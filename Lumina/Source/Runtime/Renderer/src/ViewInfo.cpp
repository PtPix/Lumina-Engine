#include "Scene/ViewInfo.h"
#include "Camera/Camera.h"
#include <DirectXMath.h>

void FViewInfo::SetupFromCamera(Camera* Camera, uint32_t RenderWidth, uint32_t RenderHeight)
{
    if (!Camera)
    {
        return;
    }

    // Extract camera matrices
    DirectX::XMMATRIX ViewMat = Camera->GetViewMatrix();
    DirectX::XMMATRIX ProjMat = Camera->GetProjectionMatrix();
    DirectX::XMMATRIX ViewProjMat = DirectX::XMMatrixMultiply(ViewMat, ProjMat);

    // Store transposed matrices for HLSL (column-major)
    DirectX::XMStoreFloat4x4(&ViewMatrix, DirectX::XMMatrixTranspose(ViewMat));
    DirectX::XMStoreFloat4x4(&ProjectionMatrix, DirectX::XMMatrixTranspose(ProjMat));
    DirectX::XMStoreFloat4x4(&ViewProjectionMatrix, DirectX::XMMatrixTranspose(ViewProjMat));

    // Store camera position
    DirectX::XMFLOAT3 CameraPos = Camera->GetPosition();
    CameraPosition = DirectX::XMFLOAT4(CameraPos.x, CameraPos.y, CameraPos.z, 1.0f);

    // Store render target dimensions
    RenderTargetWidth = RenderWidth;
    RenderTargetHeight = RenderHeight;
}