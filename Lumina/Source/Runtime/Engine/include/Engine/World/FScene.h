#pragma once
#include <DirectXMath.h>
#include <vector>

#include "FMaterial.h"
#include "SharedTypes.h"

class FMesh;
class FSceneView;

struct FGlobalPassData
{
    DirectX::XMFLOAT3 SunDirection = { 0.577f, -0.577f, 0.577f };
    float SunIntensity = 1.0f;
    DirectX::XMFLOAT4 SunColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct FDrawCommand
{
    FMesh* pMesh = nullptr;
    uint32_t InstanceIndex = 0;
};

class FGameObject
{
public:
    DirectX::XMMATRIX Transform = DirectX::XMMatrixIdentity();
    FMesh* pMesh = nullptr;
    uint32_t MaterialIndex = 0;
};

class FScene
{
public:
    FScene() = default;
    ~FScene() = default;

    void Clear();

    void AddGameObject(const FGameObject& Object);
    uint32_t AddMaterial(const FPBRMaterial& Material);

    void SetGlobalData(const FGlobalPassData& Data) { mGlobalPassData = Data; }
    FGlobalPassData& GetGlobalPassData() { return mGlobalPassData; }

    void ExtractSceneView(FSceneView& View) const;

    std::vector<FGameObject>& GetGameObjects() { return mGameObjects; }

private:
    std::vector<FGameObject> mGameObjects;
    std::vector<FPBRMaterial> mMaterials;
    FGlobalPassData mGlobalPassData;
};
