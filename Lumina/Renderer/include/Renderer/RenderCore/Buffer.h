#pragma once

#include <cstdint>
#include <string>

#include "D3D12MemAlloc.h"

// Basic Buffer Description
enum EBufferType
{
    VERTEX_BUFFER = 0,
    INDEX_BUFFER,
    CONSTANT_BUFFER,

    NUM_BUFFER_TYPES
};

struct FBufferDescriptor
{
    EBufferType Type;
    uint32_t NumElements;
    uint32_t Stride;
    const void* pData;
    std::string Name;
};

// Vertex Buffer
enum EVertexBufferType
{
    DEFAULT = 0,
    COLOR,
    COLOR_AND_ALPHA,
    NORMAL,
    NORMAL_AND_TANGENT,
    NORMAL_AND_TANGENT_PACKED1,
    NORMAL_AND_TANGENT_PACKED2,

    NUM_VERTEX_BUFFER_TYPES
};

struct FVertexDefault
{
    float position[3];
    float uv[2];
};
struct FVertexWithColor
{
    float position[3];
    float color[3];
    float uv[2];
};
struct FVertexWithColorAndAlpha
{
    float position[3];
    float color[4];
    float uv[2];
};
struct FVertexWithNormal
{
    float position[3];
    float normal[3];
    float uv[2];
};
struct FVertexWithNormalAndTangent
{
    float position[3];
    float normal[3];
    float tangent[3];
    float uv[2];
};
struct FVertexWithNormalAndTangentPacked1
{
    float position[3];
    uint16_t normal[3];
    uint16_t tangent[3];
    uint16_t uv[2];
};
struct FVertexWithNormalAndTangentPacked2
{
    float position[3];
    uint32_t normal; // rgb10 a2
    uint32_t tangent;
    uint16_t uv[2];
};

// Static Buffer Heap
class StaticBufferHeap
{
    static size_t MEMORY_ALIGNMENT;

public:
    void Create(D3D12MA::Allocator* pAllocator, EBufferType Type, uint32_t TotalMemorySize, bool bUseVidMem, const char* Name);
    void Destroy();

    bool AllocBuffer(uint32_t NumElements, uint32_t StrideInBytes, const void* pInitData, D3D12_GPU_VIRTUAL_ADDRESS* pBufferLocationOut, uint32_t* pSizeOut);
    bool AllocVertexBuffer(uint32_t NumVertices, uint32_t StrideInBytes, const void* pInitData, D3D12_VERTEX_BUFFER_VIEW* pViewOut);
    bool AllocIndexBuffer(uint32_t NumIndices, uint32_t StrideInBytes, const void* pInitData, D3D12_INDEX_BUFFER_VIEW* pViewOut);
    bool AllocConstantBuffer(uint32_t NumElements, uint32_t StrideInBytes, void** ppMappedDataOut, D3D12_GPU_VIRTUAL_ADDRESS* pGpuAddressOut);

    void UploadData(ID3D12GraphicsCommandList* pCommandList);

private:
    bool AllocBuffer(uint32_t NumElements, uint32_t StrideInBytes, void** ppDataOut, D3D12_GPU_VIRTUAL_ADDRESS* pBufferLocationOut, uint32_t* pSizeOut);
    bool AllocVertexBuffer(uint32_t NumVertices, uint32_t StrideInBytes, void** ppDataOut, D3D12_VERTEX_BUFFER_VIEW* pViewOut);
    bool AllocIndexBuffer(uint32_t NumIndices, uint32_t StrideInBytes, void** ppDataOut, D3D12_INDEX_BUFFER_VIEW* pViewOut);

private:
    D3D12MA::Allocator* mpAllocator = nullptr;
    EBufferType mBufferType = NUM_BUFFER_TYPES;

    bool mbUseVidMem = true;
    char* mpData = nullptr;
    uint32_t mMemoryInit = 0;
    uint32_t mMemoryOffset = 0;
    uint32_t mTotalMemorySize = 0;

    D3D12MA::Allocation* mpSystemMemoryAllocation = nullptr;
    D3D12MA::Allocation* mpVidMemoryAllocation = nullptr;

    ID3D12Resource* mpSystemMemoryBuffer = nullptr;
    ID3D12Resource* mpVidMemoryBuffer = nullptr;
};

struct FDynamicAllocation
{
    void* CpuAddress;
    D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;
};

class DynamicUploadHeap
{
public:
    DynamicUploadHeap() = default;
    ~DynamicUploadHeap();

    bool Initialize(D3D12MA::Allocator* pAllocator, uint32_t TotalSize = 1024 * 1024 * 8);
    void Destroy();

    FDynamicAllocation Allocate(uint32_t SizeInBytes);

    void Reset();

private:
    D3D12MA::Allocation* mpAllocation = nullptr;

    ID3D12Resource* mpUploadBuffer = nullptr;
    void* mpCpuBaseAddress = nullptr;
    D3D12_GPU_VIRTUAL_ADDRESS mGpuBaseAddress = 0;

    uint32_t mTotalSize = 0;
    uint32_t mCurrentOffset = 0;
};