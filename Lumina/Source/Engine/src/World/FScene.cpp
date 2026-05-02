#include "Engine/World/FScene.h"

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
    View.Clear();

    View.GlobalPassData = mGlobalPassData;

    View.MaterialData.reserve(mMaterials.size());
    for (const auto& Material : mMaterials)
    {
        View.MaterialData.push_back(Material.GetMaterialData());
    }

    View.InstanceData.reserve(mGameObjects.size());
    View.DrawCommands.reserve(mGameObjects.size());

    for (const auto& Object : mGameObjects)
    {
        // TODO : Culling
        uint32_t CurrentInstanceIndex = static_cast<uint32_t>(View.InstanceData.size());

        FInstanceData InstanceData;
        InstanceData.WorldMatrix = DirectX::XMMatrixTranspose(Object.Transform);
        InstanceData.MaterialIndex = Object.MaterialIndex;
        View.InstanceData.push_back(InstanceData);

        FDrawCommand DrawCommand;
        DrawCommand.pMesh = Object.pMesh;
        DrawCommand.InstanceIndex = CurrentInstanceIndex;
        View.DrawCommands.push_back(DrawCommand);
    }
}
