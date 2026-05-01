#pragma once

#include "Renderer/D3D12Core/Resource/GpuResource.h"
#include "D3D12MemAlloc.h"

class FBuffer : public GpuResource
{
public:
    FBuffer() = default;
    ~FBuffer() override { Destroy(); }
    FBuffer(const FBuffer&) = delete;
    FBuffer& operator=(const FBuffer&) = delete;
    FBuffer(FBuffer&& Other) noexcept;
    FBuffer& operator=(FBuffer&& Other) noexcept;

    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t Alignment,
        D3D12_RESOURCE_FLAGS Flags, D3D12_RESOURCE_STATES InitialState, D3D12_HEAP_TYPE HeapType,
        const wchar_t* pName);
    void Destroy();

    void* Map();
    void Unmap();

    [[nodiscard]] size_t GetBufferSize() const { return mBufferSize; }
    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return mpResource ? mpResource->GetGPUVirtualAddress() : 0; }

protected:
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> mpAllocation;
    size_t mBufferSize = 0;
};

class FVertexBuffer : public FBuffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t StrideInBytes, const wchar_t* pName = L"VertexBuffer");

    [[nodiscard]] const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return mView; }

private:
    D3D12_VERTEX_BUFFER_VIEW mView = {};
};

class FIndexBuffer : public FBuffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, DXGI_FORMAT Format, const wchar_t* pName = L"IndexBuffer");

    [[nodiscard]] const D3D12_INDEX_BUFFER_VIEW& GetView() const { return mView; }

private:
    D3D12_INDEX_BUFFER_VIEW mView = {};
};

class FConstantBuffer : public FBuffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName = L"ConstantBuffer");
    [[nodiscard]] size_t GetAlignedSize() const { return mBufferSize; }

private:
    static size_t CalcAlignedSize(size_t SizeInBytes)
    {
        return (SizeInBytes + 255) & ~255;
    }
};

class FUploadBuffer : public FBuffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName = L"UploadBuffer");
};