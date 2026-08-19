/**
 * @file Buffer.h
 * @brief GPU Buffer wrappers.
 *
 * Provides specialized classes for Vertex, Index, Constant, and Upload buffers.
 * Handles allocation via D3D12MemAlloc and manages CPU-GPU memory mapping.
 */

#pragma once

#include "Renderer/D3D12Core/Descriptors/DescriptorAllocation.h"
#include "Renderer/D3D12Core/Resource/GpuResource.h"
#include "D3D12MemAlloc.h"

class FDevice;

class FBuffer : public GpuResource
{
public:
    FBuffer() = default;
    ~FBuffer() override { Destroy(); }

    FBuffer(const FBuffer&) = delete;
    FBuffer& operator=(const FBuffer&) = delete;

    FBuffer(FBuffer&& Other) noexcept;
    FBuffer& operator=(FBuffer&& Other) noexcept;

    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t Alignment,
        D3D12_RESOURCE_FLAGS Flags, D3D12_RESOURCE_STATES InitialState, D3D12_HEAP_TYPE HeapType,
        const wchar_t* pName);
    virtual void Destroy();

    // ------------------------------------------------------------------------
    // Memory Mapping
    // ------------------------------------------------------------------------
    void* Map();
    void Unmap();

    void* GetPersistentMappedPtr();
    void ReleasePersistentMapping();

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    [[nodiscard]] size_t GetBufferSize() const { return mBufferSize; }
    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return mpResource ? mpResource->GetGPUVirtualAddress() : 0; }

protected:
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> mpAllocation;
    size_t mBufferSize = 0;

    void* mpPersistentMappedData = nullptr;
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
        // Constant buffers must be 256-byte aligned
        return (SizeInBytes + 255) & ~255;
    }
};

class FUploadBuffer : public FBuffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName = L"UploadBuffer");
};

class FStructuredBuffer : public FBuffer
{
public:
    static constexpr uint32_t InvalidBindlessIndex = UINT32_MAX;

    bool Create(FDevice* pDevice, D3D12MA::Allocator* pAllocator, uint32_t ElementSize, uint32_t ElementCount,
        bool bAllowUAV = false, bool bWithCounter = false, D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT,
        const wchar_t* pName = L"StructuredBuffer");

    void Destroy() override;

    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return mSRV.IsValid() ? mSRV.GetCpuHandle() : D3D12_CPU_DESCRIPTOR_HANDLE{}; }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetUAV() const { return mUAV.IsValid() ? mUAV.GetCpuHandle() : D3D12_CPU_DESCRIPTOR_HANDLE{}; }

    [[nodiscard]] uint32_t GetBindlessSRVIndex() const { return mBindlessSRVIndex; }
    [[nodiscard]] uint32_t GetBindlessUAVIndex() const { return mBindlessUAVIndex; }

    [[nodiscard]] uint32_t GetElementSize() const { return mElementSize; }
    [[nodiscard]] uint32_t GetElementCount() const { return mElementCount; }
    [[nodiscard]] uint64_t GetCounterOffset() const { return mCounterOffset; }

private:
    FDescriptorAllocation mSRV;
    FDescriptorAllocation mUAV;

    uint32_t mBindlessSRVIndex = InvalidBindlessIndex;
    uint32_t mBindlessUAVIndex = InvalidBindlessIndex;

    uint32_t mElementSize = 0;
    uint32_t mElementCount = 0;
    uint64_t mCounterOffset = 0;

    FDevice* mpDevice = nullptr;
};

class FByteAddressBuffer : public FBuffer
{
public:
    static constexpr uint32_t InvalidBindlessIndex = UINT32_MAX;

    bool Create(FDevice* pDevice, D3D12MA::Allocator* pAllocator, size_t SizeInBytes,
        bool bAllowUAV = false, const wchar_t* pName = L"ByteAddressBuffer");

    void Destroy() override;

    [[nodiscard]] uint32_t GetBindlessSRVIndex() const { return mBindlessSRVIndex; }
    [[nodiscard]] uint32_t GetBindlessUAVIndex() const { return mBindlessUAVIndex; }

private:
    FDescriptorAllocation mSRV;
    FDescriptorAllocation mUAV;

    uint32_t mBindlessSRVIndex = InvalidBindlessIndex;
    uint32_t mBindlessUAVIndex = InvalidBindlessIndex;

    FDevice* mpDevice = nullptr;
};