#pragma once
#include "ITestLayer.h"

#include <vector>

#include "Engine/Camera.h"
#include "Engine/World/FScene.h"
#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/D3D12Core/Resource/FTexture.h"
#include "Renderer/Resources/FMaterial.h"
#include "Renderer/Scene/Scene.h"

class FMesh;

class PBRModelTestLayer : public ITestLayer
{
public:
    std::string GetName() const override { return "PBR Model Rendering"; }

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(double DeltaTime) override;
    void OnRender(FCommandContext* pCommandContext) override;
    void OnRenderUI() override;

private:
    FScene mScene = {};
    FSceneView mSceneView = {};

    std::vector<FMesh*> mLoadedMeshes;

    // 保持映射的指针 (优化技巧)
    void* mpMappedGlobalData = nullptr;

    Camera mCamera;
    FTexture mDepthBuffer = {};
};
