#pragma once
#include <DirectXMath.h>

#include "Renderer/RenderTypes.h"
#include "Renderer/Resources/FMaterial.h"
#include "Renderer/Scene/FSceneView.h"

class FGameObject
{
public:
    DirectX::XMMATRIX Transform = DirectX::XMMatrixIdentity();
    class FMesh* pMesh = nullptr;
    uint32_t MaterialIndex = 0;
};

class FScene
{
public:
    FScene() = default;
    ~FScene() = default;

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
