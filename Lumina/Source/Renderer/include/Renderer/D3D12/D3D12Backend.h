/**
 * @file D3D12Backend.h
 * @brief Top-level DirectX 12 Backend Manager.
 *
 * Orchestrates the core D3D12 RHI (Render Hardware Interface) components
 * including the FDevice, FSwapChain, and Command Queues.
 * Acts as the primary interface for the higher-level renderer to interact with the GPU.
 */

#pragma once

#include <cstdint>
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <d3d12.h>
#include <dxgiformat.h>

class FD3D12Device;
class FD3D12SwapChain;
class FD3D12CommandQueue;
class FD3D12CommandContext;
class FD3D12DescriptorAllocator;
class FD3D12BindlessDescriptorHeap;
class D3D12GpuResource;

struct ID3D12Device;
struct IDXGIFactory6;

namespace D3D12MA { class Allocator; }

struct FD3D12BackendDesc
{
    HWND Hwnd = nullptr;

    uint32_t Width = 1280;
    uint32_t Height = 720;
    uint32_t BackBufferCount = 3;

    DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    bool bEnableDebugLayer = true;
    bool bEnableGpuValidation = false;
    bool bVSync = true;
    bool bAllowTearing = true;

    IDXGIFactory6* ExternalFactory = nullptr;
};

class FD3D12Backend
{
public:
    FD3D12Backend() = default;
    ~FD3D12Backend();

    FD3D12Backend(const FD3D12Backend&) = delete;
    FD3D12Backend& operator=(const FD3D12Backend&) = delete;
    FD3D12Backend(FD3D12Backend&&) = delete;
    FD3D12Backend& operator=(FD3D12Backend&&) = delete;

    // ------------------------------------------------------------------------
    // Lifecycle & Core Operations
    // ------------------------------------------------------------------------
    bool Initialize(const FD3D12BackendDesc& Desc);
    void Shutdown();
    void FlushAllQueues();
    void CollectGarbage();

    // ------------------------------------------------------------------------
    // Presentation & SwapChain Operations
    // ------------------------------------------------------------------------
    bool ResizeSwapChain(uint32_t Width, uint32_t Height);
    void Present();

    // ------------------------------------------------------------------------
    // Context Allocation & Execution
    // ------------------------------------------------------------------------
    [[nodiscard]] FD3D12CommandContext* AllocateGraphicsContext();
    [[nodiscard]] FD3D12CommandContext* AllocateComputeContext();
    [[nodiscard]] FD3D12CommandContext* AllocateCopyContext();

    uint64_t ExecuteGraphicsContext(FD3D12CommandContext* pCommandContext);
    uint64_t ExecuteComputeContext(FD3D12CommandContext* pCommandContext);
    uint64_t ExecuteCopyContext(FD3D12CommandContext* pCommandContext);

    // ------------------------------------------------------------------------
    // Getters - State & Configurations
    // ------------------------------------------------------------------------
    [[nodiscard]] bool IsInitialized() const { return mbInitialized; }
    [[nodiscard]] const FD3D12BackendDesc& GetDesc() const { return mDesc; }

    [[nodiscard]] uint32_t GetWidth() const { return mDesc.Width; }
    [[nodiscard]] uint32_t GetHeight() const { return mDesc.Height; }
    [[nodiscard]] uint32_t GetBackBufferCount() const { return mDesc.BackBufferCount; }
    [[nodiscard]] DXGI_FORMAT GetBackBufferFormat() const { return mDesc.BackBufferFormat; }

    [[nodiscard]] uint32_t GetCurrentBackBufferIndex() const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTV() const;
    [[nodiscard]] D3D12GpuResource* GetCurrentBackBufferResource() const;

    // ------------------------------------------------------------------------
    // Getters - Core RHI Objects
    // ------------------------------------------------------------------------
    [[nodiscard]] FD3D12Device* GetDevice() const { return mpDevice.get(); }
    [[nodiscard]] ID3D12Device* GetD3D12Device() const;
    [[nodiscard]] D3D12MA::Allocator* GetAllocator() const;

    [[nodiscard]] FD3D12CommandQueue* GetGraphicsQueue() const;
    [[nodiscard]] FD3D12CommandQueue* GetComputeQueue() const;
    [[nodiscard]] FD3D12CommandQueue* GetCopyQueue() const;

    [[nodiscard]] FD3D12DescriptorAllocator* GetSrvUavCbvAllocator() const;
    [[nodiscard]] FD3D12DescriptorAllocator* GetRtvAllocator() const;
    [[nodiscard]] FD3D12DescriptorAllocator* GetDsvAllocator() const;

    [[nodiscard]] FD3D12BindlessDescriptorHeap* GetBindlessDescriptorHeap() const;

private:
    bool CreateDevice();
    bool CreateSwapChain();
    void DestroySwapChain();

private:
    FD3D12BackendDesc mDesc = {};

    std::unique_ptr<FD3D12Device> mpDevice;
    std::unique_ptr<FD3D12SwapChain> mpSwapChain;

    bool mbInitialized = false;
};
