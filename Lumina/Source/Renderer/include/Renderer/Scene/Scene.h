#pragma once
#include <DirectXColors.h>

#include "Renderer/Rendering/Mesh.h"

class StaticModel;
struct FTexture;
class MaterialBase;

struct FDirectionalLight
{
    DirectX::XMFLOAT3 Direction;
    DirectX::XMFLOAT3 Color;
};

struct FRenderNode
{
    FMesh* pMesh = nullptr;
    MaterialBase* pMaterial = nullptr;
    DirectX::XMMATRIX ModelMatrix;
};

class FScene
{
public:
    void AddRenderNode(FMesh* pMesh, MaterialBase* pMaterial, const DirectX::XMMATRIX& Transform)
    {
        mRenderNodes.push_back({ pMesh, pMaterial, Transform });
    }

    std::vector<FRenderNode>& GetRenderNodes() { return mRenderNodes; }
    const std::vector<FRenderNode>& GetRenderNodes() const { return mRenderNodes; }

    FDirectionalLight MainLight;

    StaticModel* SkyboxModel = nullptr;
    StaticModel* CharacterModel = nullptr;

    Texture* SkyboxTexture = nullptr;
    Texture* PBRTextures[5];

    float Metallic = 0.0f;
    float Roughness = 0.0f;
    float AO = 0.0f;

private:
    std::vector<FRenderNode> mRenderNodes;
};
