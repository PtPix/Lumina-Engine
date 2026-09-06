#pragma once
#include <DirectXMath.h>
#include <vector>

struct FStandardVertex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexCoord;

    bool operator==(const FStandardVertex& other) const
    {
        return Position.x == other.Position.x && Position.y == other.Position.y && Position.z == other.Position.z &&
                TexCoord.x == other.TexCoord.x && TexCoord.y == other.TexCoord.y &&
                Normal.x == other.Normal.x && Normal.y == other.Normal.y && Normal.z == other.Normal.z;
    }
};

struct FMeshData
{
    std::vector<FStandardVertex> Vertices;
    std::vector<uint32_t> Indices;
    uint32_t MaterialIndex = 0;
};