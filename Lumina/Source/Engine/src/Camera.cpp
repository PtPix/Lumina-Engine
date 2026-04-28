#include "Engine/Camera.h"

#include <algorithm>

Camera::Camera() :
    mPosition(0.0f, 0.0f, -1.0f), mYaw(0.0f), mPitch(0.0f),
    mMovementSpeed(1.5f), mLookSpeed(0.008f),
    mbViewDirty(true),
    mFovY(DirectX::XM_PIDIV4), mAspectRatio(1280.0f / 720.0f), mNearPlane(0.1f), mFarPlane(1000.0f),
    mPendingMovement(0, 0, 0)
{
    mViewMatrix = DirectX::XMMatrixIdentity();
}

void Camera::SetLens(float FovY, float AspectRatio, float NearPlane, float FarPlane)
{
    mFovY = FovY;
    mAspectRatio = AspectRatio;
    mNearPlane = NearPlane;
    mFarPlane = FarPlane;
}

void Camera::AddMovementInput(float Right, float Up, float Forward)
{
    mPendingMovement.x += Right;
    mPendingMovement.y += Up;
    mPendingMovement.z += Forward;
}

void Camera::AddRotationInput(float DeltaYaw, float DeltaPitch)
{
    mYaw += DeltaYaw * mLookSpeed;
    mPitch += DeltaPitch * mLookSpeed;
    mPitch = std::clamp(mPitch, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);
    mbViewDirty = true;
}

void Camera::Update(double DeltaTime)
{
    if (mPendingMovement.x == 0.0f && mPendingMovement.y == 0.0f && mPendingMovement.z == 0.0f)
    {
        return;
    }

    DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f);
    DirectX::XMVECTOR ForwardVec = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), RotationMatrix);
    DirectX::XMVECTOR RightVec = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), RotationMatrix);
    DirectX::XMVECTOR UpVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR PositionVec = DirectX::XMLoadFloat3(&mPosition);
    float Speed = mMovementSpeed * static_cast<float>(DeltaTime);

    PositionVec = DirectX::XMVectorAdd(PositionVec, DirectX::XMVectorScale(RightVec, mPendingMovement.x * Speed));
    PositionVec = DirectX::XMVectorAdd(PositionVec, DirectX::XMVectorScale(UpVec, mPendingMovement.y * Speed));
    PositionVec = DirectX::XMVectorAdd(PositionVec, DirectX::XMVectorScale(ForwardVec, mPendingMovement.z * Speed));

    DirectX::XMStoreFloat3(&mPosition, PositionVec);

    mPendingMovement = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    mbViewDirty = true;
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
    if (mbViewDirty)
    {
        DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f);
        DirectX::XMVECTOR ForwardVec = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), RotationMatrix);
        DirectX::XMVECTOR PositionVec = DirectX::XMLoadFloat3(&mPosition);
        DirectX::XMVECTOR TargetVec = DirectX::XMVectorAdd(PositionVec, ForwardVec);
        DirectX::XMVECTOR UpVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        mViewMatrix = DirectX::XMMatrixLookAtLH(PositionVec, TargetVec, UpVec);
        mbViewDirty = false;
    }

    return mViewMatrix;
}

DirectX::XMMATRIX Camera::GetProjectionMatrix() const
{
    return DirectX::XMMatrixPerspectiveFovLH(mFovY, mAspectRatio, mNearPlane, mFarPlane);
}

FSceneView Camera::GetSceneView(uint32_t Width, uint32_t Height)
{
    FSceneView View;
    DirectX::XMMATRIX ViewMatrix = GetViewMatrix();
    DirectX::XMMATRIX ProjectionMatrix = GetProjectionMatrix();

    View.ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
    DirectX::XMVECTOR Det;
    View.InverseViewProjectionMatrix = DirectX::XMMatrixInverse(&Det, View.ViewProjectionMatrix);

    View.CameraPosition = mPosition;
    View.ProjectionMatrix = GetProjectionMatrix();
    View.ViewportWidth = Width;
    View.ViewportHeight = Height;

    return View;
}
