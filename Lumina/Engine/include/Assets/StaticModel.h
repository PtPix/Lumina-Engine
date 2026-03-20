#pragma once

#include "../../../Renderer/include/Renderer/RenderCore/ResourceView.h"

#include <string>
#include <vector>

#include "Renderer/Rendering/DeferredShadingRenderer.h"

class LuminaRenderer;
struct ID3D12GraphicsCommandList;

struct FSubMesh
{
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
    uint32_t IndexCount;
    uint32_t MaterialIndex;

};

class StaticModel
{
public:
    StaticModel() = default;
    ~StaticModel() = default;

    bool LoadFromFile(const std::string& FilePath, GraphicsDevice& Device);

    void Draw(ID3D12GraphicsCommandList* CommandList) const;

    const std::vector<std::string>& GetMaterialName() const { return mMaterialNames; }
    const std::vector<FSubMesh>& GetSubMeshes() const { return mSubMeshes; }

private:
    std::vector<FSubMesh> mSubMeshes;
    std::vector<std::string> mMaterialNames;
    bool mbIsLoaded = false;
};