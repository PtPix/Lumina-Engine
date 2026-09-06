/**
  * @file GPUSceneData.h
  * @brief GPU scene data structures shared between CPU and GPU.
  *
  * These structures are uploaded to GPU as StructuredBuffers and accessed
  * via Bindless indices in shaders.
  */

#pragma once

#include <DirectXMath.h>
#include <cstdint>

struct FGPUInstanceData
{
    DirectX::XMMATRIX WorldMatrix;
    DirectX::XMMATRIX WorldMatrixInverseTranspose;

    uint32_t MaterialIndex;
    uint32_t MeshIndex;
    uint32_t PrimitiveID;
    uint32_t Flags;

    DirectX::XMFLOAT3 BoundingSphereCenter;
    float BoundingSphereRadius;

    DirectX::XMFLOAT3 BoundsMin;
    float Padding0;

    DirectX::XMFLOAT3 BoundsMax;
    float Padding1;
};

struct FGPUMaterialData
{
    DirectX::XMFLOAT4 BaseColor;

    float Metallic;
    float Roughness;
    float Padding0;
    float Padding1;

    uint32_t BaseColorTextureIndex;
    uint32_t NormalTextureIndex;
    uint32_t MetallicRoughnessTextureIndex;
    uint32_t EmissiveTextureIndex;

    uint32_t Flags;
    uint32_t ShadingModel;
    uint32_t BlendMode;
    uint32_t Padding2;
};

struct FGPUPrimitiveData
{
    DirectX::XMFLOAT3 BoundsCenter;
    float BoundsRadius;

    DirectX::XMFLOAT3 BoundsMin;
    uint32_t InstanceIndex;

    DirectX::XMFLOAT3 BoundsMax;
    uint32_t PrimitiveID;

    uint32_t Flags;
    uint32_t Padding0;
    uint32_t Padding1;
    uint32_t Padding2;
};

enum class EGPUPrimitiveFlags : uint32_t
{
    None = 0,
    Visible = 1 << 0,
    CastShadow = 1 << 1,
    ReceiveShadow = 1 << 2,
    StaticMesh = 1 << 3,
    Dynamic = 1 << 4,
};

inline EGPUPrimitiveFlags operator|(EGPUPrimitiveFlags A, EGPUPrimitiveFlags B)
{
    return static_cast<EGPUPrimitiveFlags>(static_cast<uint32_t>(A) |
static_cast<uint32_t>(B));
}

inline bool HasFlag(EGPUPrimitiveFlags Value, EGPUPrimitiveFlags Flag)
{
    return (static_cast<uint32_t>(Value) & static_cast<uint32_t>(Flag)) != 0;
}