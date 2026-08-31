/**
  * @file MeshDrawCommand.h
  * @brief Mesh draw command for rendering.
  *
  * Encapsulates all parameters needed for a single DrawIndexedInstanced call.
  */

#pragma once

#include <cstdint>
#include <d3d12.h>

class FD3D12PipelineState;

// A single mesh draw command
struct FMeshDrawCommand
{
    FD3D12PipelineState* PipelineState = nullptr;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};

    uint32_t IndexCount = 0;
    uint32_t StartIndexLocation = 0;
    int32_t BaseVertexLocation = 0;

    uint32_t InstanceCount = 1;
    uint32_t StartInstanceLocation = 0;

    uint32_t RootConstants[16] = {};

    uint64_t SortKey = 0;
};

// Build sort key for draw command batching
inline uint64_t MakeSortKey(uint32_t PSOHash, uint32_t MeshID, float Depth)
{
    // PSO 32bits | Mesh 16bits | Depth 16bits
    uint64_t Key = 0;
    Key |= (static_cast<uint64_t>(PSOHash) << 32);
    Key |= (static_cast<uint64_t>(MeshID) << 16);
    Key |= (static_cast<uint64_t>(Depth * 65535.0f) & 0xFFFF);
    return Key;
}