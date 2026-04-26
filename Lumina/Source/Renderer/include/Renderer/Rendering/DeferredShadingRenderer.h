#pragma once

#include "SceneRenderTargets.h"
#include "Renderer/RenderCore/DeferredLightingMaterial.h"
#include "Renderer/D3D12Core/GraphicsDevice.h"
#include "../D3D12Core/RootSignature.h"
#include "Renderer/Scene/Scene.h"
#include "Renderer/Scene/SceneView.h"

#include <d3d12.h>

__declspec(align(256)) struct FViewUniformData
{
    DirectX::XMMATRIX ViewProjectionMatrix;
    DirectX::XMMATRIX InverseViewProjectionMatrix;
    DirectX::XMFLOAT4 CameraPosition;
    DirectX::XMFLOAT2 ScreenResolution;
    DirectX::XMFLOAT2 Padding;
};

__declspec(align(256)) struct FDirectionalLightUniformData
{
    DirectX::XMFLOAT4 LightDirection;
    DirectX::XMFLOAT4 LightColor;
};

__declspec(align(256)) struct FPrimitiveUniformData
{
    DirectX::XMMATRIX ModelMatrix;
};

__declspec(align(256)) struct FMaterialUniformData
{
    DirectX::XMFLOAT4 BaseColor;
    float Metallic;
    float Roughness;
    float AO;
    float Padding;
};

class FDeferredShadingRenderer
{
public:
    FDeferredShadingRenderer() = default;

    bool Initialize(GraphicsDevice* pGraphicsDevice, uint32_t Width, uint32_t Height);

    void Render(FCommandContext* pContext, const FSceneView& View, const FScene& Scene);

    void Destroy();

    RootSignature* GetRootSignature() { return &mGlobalRootSignature; }

private:
    bool CreateGlobalRootSignature();
    bool InitMaterials();

    void RenderPass(FCommandContext* pCommandContext, const FSceneView& View, const FScene& Scene, ERenderPass Pass);
    void RenderDeferredLighting(FCommandContext* pCommandContext, const FSceneView& View, const FScene& Scene);

private:
    GraphicsDevice* mGraphicsDevice = nullptr;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;

    RootSignature mGlobalRootSignature;
    SceneRenderTargets mSceneRenderTargets;

    DeferredLightingMaterial mDeferredLightingMaterial;

    // ResourceView mHelmetPBRTable;
};