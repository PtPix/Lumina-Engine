#include "Engine/World/FScene.h"
#include "FMesh.h"

void FScene::Clear()
{
    mGameObjects.clear();
    mMaterials.clear();
}

void FScene::AddGameObject(const FGameObject& Object)
{
    mGameObjects.push_back(Object);
}

uint32_t FScene::AddMaterial(const FPBRMaterial& Material)
{
    uint32_t MaterialIndex = static_cast<uint32_t>(mMaterials.size());

    FPBRMaterial NewMaterial = Material;
    NewMaterial.SetMaterialID(MaterialIndex);
    mMaterials.push_back(NewMaterial);

    return MaterialIndex;
}

void FScene::ExtractSceneView(FSceneView& View) const
{
    // TODO: Implement scene view extraction when FSceneView is properly defined
    // This requires coordination with the Renderer module
}
