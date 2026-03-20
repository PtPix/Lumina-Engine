#pragma once
#include <DirectXColors.h>

class StaticModel;
struct FTexture;

struct FDirectionalLight
{
    DirectX::XMFLOAT3 Direction;
    DirectX::XMFLOAT3 Color;
};

struct FScene
{
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
};
