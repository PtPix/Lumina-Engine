#pragma once

#include "IRenderDemo.h"
#include "Camera/Camera.h"
#include "Scene/Scene.h"

class FMesh;

class PbrModelDemo : public IRenderDemo
{
public:
    std::string GetName() const override { return "PBR Model"; }
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(double DeltaTime) override;
    void OnRender(FRenderGraph& Graph) override;
    void OnRenderUI() override;

private:
    struct FParams
    {
        float SunDirection[3] = { 0.577f, -0.577f, 0.577f };
        float SunColor[3]     = { 1.0f, 0.9f, 0.8f };
        float SunIntensity    = 3.14f;
        bool  bRotateModel    = false;
    } mParams;

    FScene mScene = {};
    Camera mCamera;
    std::vector<FMesh*> mLoadedMeshes;
};
