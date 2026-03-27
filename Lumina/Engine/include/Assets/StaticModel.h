#pragma once

#include "../../../Renderer/include/Renderer/RenderCore/ResourceView.h"

#include <string>
#include <vector>

#include "Renderer/Rendering/DeferredShadingRenderer.h"

class LuminaRenderer;
struct ID3D12GraphicsCommandList;

struct FSubMesh
{
    void* Vertices;
    void* Indices;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
    uint32_t IndexCount;
    uint32_t MaterialIndex;
};

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

    bool LoadFromFile(const std::string& FilePath, GraphicsDevice& Device);

    void SumbitToScene(FScene* pScene, const DirectX::XMMATRIX& Transform);

    void Draw(ID3D12GraphicsCommandList* CommandList) const;

    const std::vector<std::string>& GetMaterialName() const { return mMaterialNames; }
    const std::vector<FSubMesh>& GetSubMeshes() const { return mSubMeshes; }

private:
    std::vector<FMesh> mMeshes;
    std::vector<FSubMesh> mSubMeshes;
    std::vector<std::string> mMaterialNames;
    bool mbIsLoaded = false;
};