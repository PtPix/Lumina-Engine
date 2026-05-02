#pragma once
#include "ITestLayer.h"

#include <vector>

#include "Engine/Camera.h"
#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/D3D12Core/Resource/FTexture.h"
#include "Renderer/Resources/FMaterial.h"
#include "Renderer/Scene/Scene.h"

class FMesh;

struct alignas(16) FInstanceData
{
    DirectX::XMMATRIX WorldMatrix;
    uint32_t MaterialIndex;
    uint32_t Pad[3];
};

class PBRModelTestLayer : public ITestLayer
{
public:
    std::string GetName() const override { return "PBR Model Rendering"; }

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(double DeltaTime) override;
    void OnRender(FCommandContext* pCommandContext) override;
    void OnRenderUI() override;

    void CreateInstanceInScene(FMesh* pMesh, DirectX::XMMATRIX Transform, uint32_t MaterialID);
    bool InitBasePassPipeline();

private:
    // Object
    std::vector<FSceneObject> mSceneObjects;
    std::vector<FMesh*> mLoadedMeshes;

    // Data
    std::vector<FPBRMaterial> mSceneMaterials;
    std::vector<FPBRMaterialData> mMaterialCache;
    std::vector<FInstanceData> mInstanceData;

    // Buffer
    FBuffer mMaterialBuffer;
    FBuffer mInstanceBuffer;

    PipelineState mBasePassPSO;
    FGlobalPassData mGlobalDataCache = {};

    // 对应的 GPU 资源
    FConstantBuffer mGlobalPassBuffer;

    // 保持映射的指针 (优化技巧)
    void* mpMappedGlobalData = nullptr;

    Camera mCamera;
    FTexture mDepthBuffer = {};
};
