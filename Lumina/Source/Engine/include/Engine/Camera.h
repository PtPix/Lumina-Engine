#pragma once

#include <DirectXMath.h>
#include <windows.h>

#include "Renderer/Scene/FSceneView.h"

class Camera
{
public:
    Camera();

    void SetLens(float FovY, float AspectRatio, float NearPlane, float FarPlane);

    void AddMovementInput(float Right, float Up, float Forward);
    void AddRotationInput(float DeltaYaw, float DeltaPitch);
    void Update(double DeltaTime);

    [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const;
    [[nodiscard]] DirectX::XMMATRIX GetProjectionMatrix() const;
    [[nodiscard]] DirectX::XMFLOAT3 GetPosition() const { return mPosition; }

private:
    DirectX::XMFLOAT3 mPosition;
    float mYaw; float mPitch;
    float mMovementSpeed; float mLookSpeed;

    mutable DirectX::XMMATRIX mViewMatrix;
    mutable bool mbViewDirty;

    float mFovY; float mAspectRatio;
    float mNearPlane; float mFarPlane;

    DirectX::XMFLOAT3 mPendingMovement;
};
