#pragma once

#include <DirectXMath.h>
#include <functional>
#include <string>

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

namespace std
{
    template<> struct hash<FStandardVertex>
    {
        size_t operator()(const FStandardVertex& vertex) const noexcept
        {
            return ((hash<float>()(vertex.Position.x) ^
                   (hash<float>()(vertex.Position.y) << 1)) >> 1) ^
                   (hash<float>()(vertex.Position.z) << 1);
        }
    };
}

class ModelLoader
{
public:
    static bool LoadOBJ(const std::string& ModelPath, std::vector<FStandardVertex>& OutVertices, std::vector<uint32_t>& OutIndices);
};