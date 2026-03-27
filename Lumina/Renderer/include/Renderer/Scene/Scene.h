#pragma once
#include <DirectXColors.h>

#include "Renderer/Rendering/Mesh.h"

class StaticModel;
struct FTexture;

struct FDirectionalLight
{
    DirectX::XMFLOAT3 Direction;
    DirectX::XMFLOAT3 Color;
};

struct FRenderNode
{
    FMesh* pMesh = nullptr;
    Material* pMaterial = nullptr;
    DirectX::XMMATRIX ModelMatrix;
};

class FScene
{
public:
    void AddRenderNode(FMesh* pMesh, Material* pMaterial, const DirectX::XMMATRIX& Transform)
    {
        mRenderNodes.push_back({ pMesh, pMaterial, Transform });
    }

    const std::vector<FRenderNode>& GetRenderNodes() const { return mRenderNodes; }

    FDirectionalLight MainLight;

    StaticModel* SkyboxModel = nullptr;
    StaticModel* CharacterModel = nullptr;

    FTexture* SkyboxTexture = nullptr;
    FTexture* PBRTextures[5] = { nullptr };

    D3D12_GPU_DESCRIPTOR_HANDLE SkyboxSrvTable;
    D3D12_GPU_DESCRIPTOR_HANDLE HelmetPBRSrvTable;

    float Metallic = 0.0f;
    float Roughness = 0.0f;
    float AO = 0.0f;

    ResourceView PBRViews[5];
private:
    std::vector<FRenderNode> mRenderNodes;
};
