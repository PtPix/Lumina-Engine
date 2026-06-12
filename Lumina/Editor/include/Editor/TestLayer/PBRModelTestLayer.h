#pragma once
#include "ITestLayer.h"

#include <vector>

#include "Engine/Camera.h"
#include "Engine/World/FScene.h"
#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/D3D12Core/Resource/FTexture.h"
#include "Renderer/Resources/FMaterial.h"

class FMesh;

class PBRModelTestLayer : public ITestLayer
{
public:
    std::string GetName() const override { return "PBR Model Rendering"; }

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(double DeltaTime) override;
    void OnRender(FRenderGraph& RenderGraph) override;
    void OnRenderUI() override;

private:
    FScene mScene = {};
    FSceneView mSceneView = {};

    std::vector<FMesh*> mLoadedMeshes;

    Camera mCamera;
};
