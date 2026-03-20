#include "Assets/StaticModel.h"

#include "Assets/ModelLoader.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Logger/Logger.h"

bool StaticModel::LoadFromFile(const std::string& FilePath, GraphicsDevice& Device)
{
    Assimp::Importer Importer;

    unsigned int ImportFlags =
            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded;
    const aiScene* pScene = Importer.ReadFile(FilePath, ImportFlags);

    if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
    {
        Log::Error("[StaticModel] Assimp Failed to load %s: %s", FilePath.c_str(), Importer.GetErrorString());
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

        // Generate SubMesh
        FSubMesh SubMesh = {};
        SubMesh.IndexCount = static_cast<uint32_t>(Indices.size());
        SubMesh.MaterialIndex = pAiMesh->mMaterialIndex;

        // Allocate buffer and record upload command
        Device.GetVertexBufferHeap().AllocVertexBuffer(
            static_cast<uint32_t>(Vertices.size()), sizeof(FStandardVertex), Vertices.data(), &SubMesh.VertexBufferView
            );
        Device.GetIndexBufferHeap().AllocIndexBuffer(
            SubMesh.IndexCount, sizeof(uint32_t), Indices.data(), &SubMesh.IndexBufferView
            );
        mSubMeshes.push_back(SubMesh);
    }

    Device.GetVertexBufferHeap().UploadData(pUploadCommandList);
    Device.GetIndexBufferHeap().UploadData(pUploadCommandList);

    mbIsLoaded = true;
    Log::Info("Loaded %s [SubMeshes: %zu, Materials: %zu]", FilePath.c_str(), mSubMeshes.size(), mMaterialNames.size());
    return true;
}

void StaticModel::Draw(ID3D12GraphicsCommandList* CommandList) const
{
    if (!mbIsLoaded) return;

    for (const FSubMesh& SubMesh : mSubMeshes)
    {
        CommandList->IASetVertexBuffers(0, 1, &SubMesh.VertexBufferView);
        CommandList->IASetIndexBuffer(&SubMesh.IndexBufferView);
        CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, 0, 0, 0);
    }
}
