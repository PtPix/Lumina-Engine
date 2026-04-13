#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>

#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

class FMesh;
class FScene;
class GraphicsDevice;

struct FStandardVertex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT2 TexCoord;
    DirectX::XMFLOAT3 Normal;

    bool operator==(const FStandardVertex& other) const
    {
        return Position.x == other.Position.x && Position.y == other.Position.y && Position.z == other.Position.z &&
                TexCoord.x == other.TexCoord.x && TexCoord.y == other.TexCoord.y &&
                Normal.x == other.Normal.x && Normal.y == other.Normal.y && Normal.z == other.Normal.z;
    }
};

class StaticModel
{
public:
    StaticModel() = default;
    ~StaticModel() = default;

    bool LoadFromFile(const std::string& FilePath, GraphicsDevice& Device, ResourceUploader* pUploader);

    size_t SumbitToScene(FScene* pScene, const DirectX::XMMATRIX& Transform);

    [[nodiscard]] const std::vector<std::string>& GetMaterialName() const { return mMaterialNames; }

private:
    std::vector<FMesh> mMeshes;
    std::vector<std::string> mMaterialNames;
};