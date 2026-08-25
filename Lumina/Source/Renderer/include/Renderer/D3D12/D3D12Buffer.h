/**
 * @file D3D12Buffer.h
 * @brief GPU Buffer wrappers.
 *
 * Provides specialized classes for Vertex, Index, Constant, and Upload buffers.
 * Handles allocation via D3D12MemAlloc and manages CPU-GPU memory mapping.
 */

#pragma once

#include "D3D12DescriptorAllocation.h"
#include "D3D12GpuResource.h"
#include "D3D12MemAlloc.h"

class FD3D12Device;

class FD3D12Buffer : public FD3D12GpuResource
{
public:
    FD3D12Buffer() = default;
    ~FD3D12Buffer() override { Destroy(); }

    FD3D12Buffer(const FD3D12Buffer&) = delete;
    FD3D12Buffer& operator=(const FD3D12Buffer&) = delete;

    FD3D12Buffer(FD3D12Buffer&& Other) noexcept;
    FD3D12Buffer& operator=(FD3D12Buffer&& Other) noexcept;

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

class FD3D12VertexBuffer : public FD3D12Buffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, size_t StrideInBytes, const wchar_t* pName = L"VertexBuffer");

    [[nodiscard]] const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return mView; }

private:
    D3D12_VERTEX_BUFFER_VIEW mView = {};
};

class FD3D12IndexBuffer : public FD3D12Buffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, DXGI_FORMAT Format, const wchar_t* pName = L"IndexBuffer");

    [[nodiscard]] const D3D12_INDEX_BUFFER_VIEW& GetView() const { return mView; }

private:
    D3D12_INDEX_BUFFER_VIEW mView = {};
};

class FD3D12ConstantBuffer : public FD3D12Buffer
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

class FD3D12UploadBuffer : public FD3D12Buffer
{
public:
    bool Create(D3D12MA::Allocator* pAllocator, size_t SizeInBytes, const wchar_t* pName = L"UploadBuffer");
};

class FD3D12StructuredBuffer : public FD3D12Buffer
{
public:
    static constexpr uint32_t InvalidBindlessIndex = UINT32_MAX;

    bool Create(FD3D12Device* pDevice, D3D12MA::Allocator* pAllocator, uint32_t ElementSize, uint32_t ElementCount,
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
    FD3D12DescriptorAllocation mSRV;
    FD3D12DescriptorAllocation mUAV;

    uint32_t mBindlessSRVIndex = InvalidBindlessIndex;
    uint32_t mBindlessUAVIndex = InvalidBindlessIndex;

    uint32_t mElementSize = 0;
    uint32_t mElementCount = 0;
    uint64_t mCounterOffset = 0;

    FD3D12Device* mpDevice = nullptr;
};

class FD3D12ByteAddressBuffer : public FD3D12Buffer
{
public:
    static constexpr uint32_t InvalidBindlessIndex = UINT32_MAX;

    bool Create(FD3D12Device* pDevice, D3D12MA::Allocator* pAllocator, size_t SizeInBytes,
        bool bAllowUAV = false, const wchar_t* pName = L"ByteAddressBuffer");

    void Destroy() override;

    [[nodiscard]] uint32_t GetBindlessSRVIndex() const { return mBindlessSRVIndex; }
    [[nodiscard]] uint32_t GetBindlessUAVIndex() const { return mBindlessUAVIndex; }

private:
    FD3D12DescriptorAllocation mSRV;
    FD3D12DescriptorAllocation mUAV;

    uint32_t mBindlessSRVIndex = InvalidBindlessIndex;
    uint32_t mBindlessUAVIndex = InvalidBindlessIndex;

    FD3D12Device* mpDevice = nullptr;
};