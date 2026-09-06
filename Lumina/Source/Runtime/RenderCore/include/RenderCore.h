/**
  * @file RenderCore.h
  * @brief Global renderer access point and lifecycle management.
  *
  * Provides static accessors to core renderer subsystems.
  * This is the central hub for accessing Device, Backend, and other global systems.
  */

#pragma once

#include "RenderTypes.h"
#include <cstdint>
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// Forward declarations
class FD3D12Backend;
class FD3D12Device;
class FD3D12CommandContext;
class FD3D12ResourceUploader;
class FGPUScene;
class FRenderGraph;
class FMesh;
struct FMeshData;
struct FRGTextureHandle;
namespace D3D12MA { class Allocator; }

struct FRendererInitParams
{
    HWND WindowHandle = nullptr;
    uint32_t Width = 1920;
    uint32_t Height = 1080;
    bool bEnableDebugLayer = true;
    bool bEnableGpuValidation = false;
    bool bVSync = false;
};

class FRendererCore
{
public:
    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    static bool Initialize(const FRendererInitParams& Params);
    static void Shutdown();

    [[nodiscard]] static bool IsInitialized() { return mbInitialized; }

    // ------------------------------------------------------------------------
    // Frame Management
    // ------------------------------------------------------------------------
    static void BeginFrame();
    static void EndFrame();

    [[nodiscard]] static uint32_t GetFrameIndex() { return mCurrentFrameIndex; }
    [[nodiscard]] static uint64_t GetFrameNumber() { return mFrameNumber; }

    // ------------------------------------------------------------------------
    // Core Subsystem Access
    // ------------------------------------------------------------------------
    [[nodiscard]] static FD3D12Backend* GetBackend() { return mpBackend.get(); }
    [[nodiscard]] static FD3D12Device* GetDevice();
    [[nodiscard]] static D3D12MA::Allocator* GetAllocator();
    [[nodiscard]] static FD3D12ResourceUploader* GetUploader() { return mpUploader.get(); }
    [[nodiscard]] static FGPUScene* GetGPUScene() { return mpGPUScene; }

    // ------------------------------------------------------------------------
    // RenderGraph Access
    // ------------------------------------------------------------------------
    [[nodiscard]] static FRenderGraph* GetRenderGraph();
    [[nodiscard]] static FRGTextureHandle GetBackBufferHandle();

    // ------------------------------------------------------------------------
    // Window Management
    // ------------------------------------------------------------------------
    static void OnResize(uint32_t Width, uint32_t Height);

    // ------------------------------------------------------------------------
    // Viewport & Resolution
    // ------------------------------------------------------------------------
    [[nodiscard]] static uint32_t GetRenderWidth() { return mRenderWidth; }
    [[nodiscard]] static uint32_t GetRenderHeight() { return mRenderHeight; }

    static void SetRenderResolution(uint32_t Width, uint32_t Height);

    // ------------------------------------------------------------------------
    // Statistics
    // ------------------------------------------------------------------------
    [[nodiscard]] static const FRenderStats& GetStats() { return mStats; }
    static void ResetStats() { mStats.Reset(); }

private:
    static void InitializeBackend(const FRendererInitParams& Params);
    static void InitializeBindlessRootSignature();

    static bool mbInitialized;

    // Backend
    static std::unique_ptr<FD3D12Backend> mpBackend;
    static std::unique_ptr<FD3D12ResourceUploader> mpUploader;

    // RenderGraph
    static std::unique_ptr<FRenderGraph> mpRenderGraph;
    static FRGTextureHandle mBackBufferHandle;

    // GPU Scene
    static FGPUScene* mpGPUScene;

    // Frame Context
    static FD3D12CommandContext* mpCurrentFrameContext;

    // Frame State
    static uint32_t mCurrentFrameIndex;
    static uint64_t mFrameNumber;

    static uint32_t mRenderWidth;
    static uint32_t mRenderHeight;

    static FRenderStats mStats;
};

// Convenience macros
#define GRendererCore       FRendererCore
#define GRenderGraph()      FRendererCore::GetRenderGraph()
#define GDevice()           FRendererCore::GetDevice()
#define GBackend()          FRendererCore::GetBackend()
#define GAllocator()        FRendererCore::GetAllocator()
#define GUploader()         FRendererCore::GetUploader()
#define GGPUScene()         FRendererCore::GetGPUScene()