#pragma once

#include "Renderer/ShaderInterop/SharedTypes.h"
#include <DirectXMath.h>
#include <string>

// struct alignas(16) FPBRMaterialData
// {
//     DirectX::XMFLOAT4 BaseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
//     DirectX::XMFLOAT4 EmissiveFactor = { 0.0f, 0.0f, 0.0f, 0.0f };
//
//     float MetallicFactor = 1.0f;
//     float RoughnessFactor = 1.0f;
//     float AlphaCutoff = 0.5f;
//     uint32_t ShadingModel = SHADINGMODEL_DEFAULT_LIT;
//
//     uint32_t AlbedoTexIndex = 0;
//     uint32_t NormalTexIndex = 0;
//     uint32_t ORMTexIndex = 0;
//     uint32_t EmissiveTexIndex = 0;
//
//     uint32_t MaterialFlags = 0;
//     uint32_t Pad[3] = {0, 0, 0};
// };

class FPBRMaterial
{
public:
    FPBRMaterial() = default;
    ~FPBRMaterial() = default;

    const FPBRMaterialData& GetMaterialData() const { return mMaterialData; }

    void SetAlbedoTexture(uint32_t AlbedoTexIndex) { mMaterialData.AlbedoTexIndex = AlbedoTexIndex; }
    void SetNormalTexture(uint32_t NormalTexIndex) { mMaterialData.NormalTexIndex = NormalTexIndex; }
    void SetORMTexture(uint32_t ORMTexIndex) { mMaterialData.ORMTexIndex = ORMTexIndex; }
    void SetEmissiveTexture(uint32_t EmissiveTexIndex) { mMaterialData.EmissiveTexIndex = EmissiveTexIndex; }

    uint32_t GetMaterialID() const { return mMaterialID; }
    void SetMaterialID(uint32_t materialID) { mMaterialID = materialID; }

private:
    std::string mName;
    uint32_t mMaterialID = 0;

    FPBRMaterialData mMaterialData;
};