#include "Assets/StaticModel.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

bool StaticModel::LoadFromFile(const std::string& FilePath)
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
    mMeshesData.clear();
    for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
    {
        aiMesh* pAiMesh = pScene->mMeshes[i];
        FMeshData MeshData;
        MeshData.MaterialIndex = pAiMesh->mMaterialIndex;

        // Get Vertex Data
        MeshData.Vertices.reserve(pAiMesh->mNumVertices);
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
            MeshData.Vertices.push_back(Vertex);
        }

        // Get Index Data
        MeshData.Indices.reserve(pAiMesh->mNumFaces * 3);
        for (unsigned int j = 0; j < pAiMesh->mNumFaces; j++)
        {
            aiFace Face = pAiMesh->mFaces[j];
            for (unsigned int k = 0; k < Face.mNumIndices; k++)
            {
                MeshData.Indices.push_back(Face.mIndices[k]);
            }
        }

        mMeshesData.push_back(std::move(MeshData));
    }

    LUMINA_LOG_INFO(StaticModel, "Loaded %s [Meshes: %zu, Materials: %zu]", FilePath.c_str(), mMeshesData.size(), mMaterialNames.size());
    return true;
}
