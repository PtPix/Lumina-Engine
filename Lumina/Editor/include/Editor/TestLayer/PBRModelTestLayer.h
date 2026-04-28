#pragma once
#include "ITestLayer.h"

#include <vector>

#include "Engine/Camera.h"
#include "Renderer/D3D12Core/PipelineState.h"
#include "Renderer/D3D12Core/Resource/FTexture.h"
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

    void CreateInstanceInScene(FMesh* pMesh, DirectX::XMMATRIX Transform, DirectX::XMFLOAT4 Color);
    bool InitBasePassPipeline();
private:
    std::vector<FSceneObject> mSceneObjects;
    PipelineState mBasePassPSO;

    FGlobalPassData mGlobalDataCache = {};

    // 对应的 GPU 资源
    FConstantBuffer mGlobalPassBuffer;

    // 保持映射的指针 (优化技巧)
    void* mpMappedGlobalData = nullptr;

    Camera mCamera;
    FTexture mDepthBuffer = {};
};
