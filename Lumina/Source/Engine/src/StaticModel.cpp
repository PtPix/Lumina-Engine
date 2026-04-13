#include "Assets/StaticModel.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Renderer/D3D12Core/GraphicsDevice.h"
#include "Renderer/Scene/Scene.h"

#include "Logger/Logger.h"

bool StaticModel::LoadFromFile(const std::string& FilePath, GraphicsDevice& Device, ResourceUploader* pUploader)
{
    Assimp::Importer Importer;

    unsigned int ImportFlags =
            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded;
    const aiScene* pScene = Importer.ReadFile(FilePath, ImportFlags);

    if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
    {
        LUMINA_LOG_ERROR(StaticModel, "Assimp Failed to load %s: %s", FilePath.c_str(), Importer.GetErrorString());
        return false;
    }

    // Get material name
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
    {
        aiMaterial* pMaterial = pScene->mMaterials[i];
        aiString MaterialName;
        pMaterial->Get(AI_MATKEY_NAME, MaterialName);
        mMaterialNames.push_back(MaterialName.C_Str());
    }

    // Get SubMeshes
    ID3D12GraphicsCommandList* pUploadCommandList = Device.GetUploadHeap().GetCommandList();
    for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
    {
        aiMesh* pAiMesh = pScene->mMeshes[i];
        std::vector<FStandardVertex> Vertices;
        std::vector<uint32_t> Indices;

        // Get Vertices data
        for (unsigned int j = 0; j < pAiMesh->mNumVertices; j++)
        {
            FStandardVertex Vertex = {};
            Vertex.Position = { pAiMesh->mVertices[j].x, pAiMesh->mVertices[j].y, pAiMesh->mVertices[j].z };

            if (pAiMesh->HasNormals())
            {
                Vertex.Normal = { pAiMesh->mNormals[j].x, pAiMesh->mNormals[j].y, pAiMesh->mNormals[j].z };
            }
            if (pAiMesh->mTextureCoords[0])
            {
                Vertex.TexCoord = { pAiMesh->mTextureCoords[0][j].x, pAiMesh->mTextureCoords[0][j].y };
            }
            Vertices.push_back(Vertex);
        }

        // Get Index data
        for (unsigned int j = 0; j < pAiMesh->mNumFaces; j++)
        {
            aiFace Face = pAiMesh->mFaces[j];
            for (unsigned int k = 0; k < Face.mNumIndices; k++)
            {
                Indices.push_back(Face.mIndices[k]);
            }
        }

        FMesh Mesh = {};
        Mesh.Initialize(&Device.GetDevice(), pUploader, Vertices.data(), sizeof(FStandardVertex), static_cast<UINT>(Vertices.size()), Indices.data(), static_cast<UINT>(Indices.size()));
        mMeshes.push_back(Mesh);
    }

    LUMINA_LOG_INFO(StaticModel, "Loaded %s [Meshes: %zu, Materials: %zu]", FilePath.c_str(), mMeshes.size(), mMaterialNames.size());
    return true;
}

size_t StaticModel::SumbitToScene(FScene* pScene, const DirectX::XMMATRIX& Transform)
{
    size_t StartIndex = pScene->GetRenderNodes().size();
    for (auto& Mesh : mMeshes)
    {
        pScene->AddRenderNode(&Mesh, nullptr, Transform);
    }
    return StartIndex;
}