#include "Assets/ModelLoader.h"
#include "Logger/Logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "Assets/tiny_obj_loader.h"

#include <unordered_map>

bool ModelLoader::LoadOBJ(const std::string& ModelPath, std::vector<FStandardVertex>& OutVertices,
    std::vector<uint32_t>& OutIndices)
{
    tinyobj::attrib_t Attrib;
    std::vector<tinyobj::shape_t> Shapes;
    std::vector<tinyobj::material_t> Materials;
    std::string Error;

    // Load from tiny_obj
    bool bSuccess = tinyobj::LoadObj(&Attrib, &Shapes, &Materials, &Error, ModelPath.c_str());
    if (!Error.empty())
    {
        Log::Info("TinyObjLoader Error: %s", Error.c_str());
    }
    if (!bSuccess)
    {
        return false;
    }

    std::unordered_map<FStandardVertex, uint32_t> UniqueVertices;

    for (const auto& Shape : Shapes)
    {
        for (const auto& Index : Shape.mesh.indices)
        {
            FStandardVertex Vertex = {};
            Vertex.Position = {
                Attrib.vertices[3 * Index.vertex_index + 0],
                Attrib.vertices[3 * Index.vertex_index + 1],
                Attrib.vertices[3 * Index.vertex_index + 2],
            };
            // Load UV if exists
            if (Index.texcoord_index >= 0)
            {
                Vertex.TexCoord = {
                    Attrib.texcoords[2 * Index.texcoord_index + 0],
                    1.0f - Attrib.texcoords[2 * Index.texcoord_index + 1]
                };
            }
            // Load Normal if exists
            if (Index.normal_index >= 0)
            {
                Vertex.Normal = {
                    Attrib.normals[3 * Index.normal_index + 0],
                    Attrib.normals[3 * Index.normal_index + 1],
                    Attrib.normals[3 * Index.normal_index + 2],
                };
            }

            // Check unique
            if (UniqueVertices.count(Vertex) == 0)
            {
                UniqueVertices[Vertex] = static_cast<uint32_t>(OutVertices.size());
                OutVertices.push_back(Vertex);
            }
            OutIndices.push_back(UniqueVertices[Vertex]);
        }
    }

    Log::Info("Model Loaded: %s | Vertices: %zu | Indices: %zu", ModelPath.c_str(), OutVertices.size(), OutIndices.size());
    return true;
}
