#pragma once

#include "Engine/LuminaApp.h"
#include "Renderer/RenderCore/TextureManager.h"
#include "Renderer/Rendering/DeferredShadingRenderer.h"

#include <d3d12.h>
#include <DirectXMath.h>

class ModelApp : public LuminaApp
{
public:
    bool OnInit() override;
    void OnUpdate(double DeltaTime) override;
    void OnRender(ID3D12GraphicsCommandList* pCommandList) override;
    void OnRenderUI() override;
    void OnDestroy() override;

private:
    FDeferredShadingRenderer mRenderer = {};
    FScene mScene = {};
    FSceneView mView = {};

    ResourceView mHelmetPBRTable;
    TextureManager mTextureManager;

    DirectX::XMFLOAT3 mLightDirection = {1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 mLightColor = {1.0f, 1.0f, 1.0f};

    float mMetallic = 1.0f;
    float mRoughness = 1.0f;
    float mAO = 1.0f;
};
