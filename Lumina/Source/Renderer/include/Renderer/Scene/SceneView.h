/**
   * @file SceneView.h
   * @brief Scene view definition (camera + render settings).
   *
   * Pure input data from game thread. Contains camera parameters and rendering configuration.
   * Does NOT contain GPU data or draw commands.
   */

#pragma once

#include "Renderer/Core/RenderTypes.h"
#include <DirectXMath.h>

// View Initialization parameters
struct FSceneViewInitOptions
{
   DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixIdentity();
   DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixIdentity();

   DirectX::XMFLOAT3 ViewLocation = { 0.0f, 0.0f, 0.0f };
   DirectX::XMFLOAT3 ViewDirection = { 0.0f, 0.0f, 1.0f };

   uint32_t ViewportWidth = 1920;
   uint32_t ViewportHeight = 1080;

   float FieldOfView = 90.0f;
   float NearClipPlane = 0.1f;
   float FarClipPlane = 10000.0f;
};

// Scene view representing a camera's perspective ( Game Thread )
class FSceneView
{
public:
   FSceneView() = default;
   explicit FSceneView(const FSceneViewInitOptions& InitOptions);

   // Camera Matrices
   void SetViewMatrix(const DirectX::XMMATRIX& ViewMatrix) { mViewMatrix = ViewMatrix; }
   void SetProjectionMatrix(const DirectX::XMMATRIX& ProjectionMatrix) { mProjectionMatrix = ProjectionMatrix; }

   [[nodiscard]] const DirectX::XMMATRIX& GetViewMatrix() const { return mViewMatrix; }
   [[nodiscard]] const DirectX::XMMATRIX& GetProjectionMatrix() const { return mProjectionMatrix; }
   [[nodiscard]] DirectX::XMMATRIX GetViewProjectionMatrix() const;
   [[nodiscard]] DirectX::XMMATRIX GetInvViewMatrix() const;
   [[nodiscard]] DirectX::XMMATRIX GetInvProjectionMatrix() const;

   // Camera Properties
   [[nodiscard]] const DirectX::XMFLOAT3& GetViewLocation() const { return mViewLocation; }
   [[nodiscard]] const DirectX::XMFLOAT3& GetViewDirection() const { return mViewDirection; }

   void SetViewLocation(const DirectX::XMFLOAT3& Location) { mViewLocation = Location; }
   void SetViewDirection(const DirectX::XMFLOAT3& Direction) { mViewDirection = Direction; }

   // Viewport
   [[nodiscard]] uint32_t GetViewportWidth() const { return mViewportWidth; }
   [[nodiscard]] uint32_t GetViewportHeight() const { return mViewportHeight; }
   [[nodiscard]] float GetAspectRatio() const
   {
      return static_cast<float>(mViewportWidth) / static_cast<float>(mViewportHeight);
   }

   void SetViewportSize(uint32_t Width, uint32_t Height)
   {
      mViewportWidth = Width;
      mViewportHeight = Height;
   }

   // Projection Parameters
   [[nodiscard]] float GetFieldOfView() const { return mFieldOfView; }
   [[nodiscard]] float GetNearClipPlane() const { return mNearClipPlane; }
   [[nodiscard]] float GetFarClipPlane() const { return mFarClipPlane; }

   void SetFieldOfView(float FovDegrees) { mFieldOfView = FovDegrees; }
   void SetClipPlanes(float Near, float Far)
   {
      mNearClipPlane = Near;
      mFarClipPlane = Far;
   }

private:
   DirectX::XMMATRIX mViewMatrix = DirectX::XMMatrixIdentity();
   DirectX::XMMATRIX mProjectionMatrix = DirectX::XMMatrixIdentity();

   DirectX::XMFLOAT3 mViewLocation = { 0.0f, 0.0f, 0.0f };
   DirectX::XMFLOAT3 mViewDirection = { 0.0f, 0.0f, 1.0f };

   uint32_t mViewportWidth = 1920;
   uint32_t mViewportHeight = 1080;

   float mFieldOfView = 90.0f;
   float mNearClipPlane = 0.1f;
   float mFarClipPlane = 10000.0f;
};