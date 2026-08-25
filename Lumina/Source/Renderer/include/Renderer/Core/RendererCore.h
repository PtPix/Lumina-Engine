/**
  * @file RendererCore.h
  * @brief Global renderer access point and lifecycle management.
  *
  * Provides static accessors to core renderer subsystems.
  * This is the central hub for accessing Device, Backend, and other global systems.
  */

#pragma once

#include "RenderTypes.h"
#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

class FD3D12Backend;
class FD3D12Device;
class FD3D12CommandContext;
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
    [[nodiscard]] static FD3D12Backend* GetBackend() { return mpBackend; }
    [[nodiscard]] static FD3D12Device* GetDevice();
     [[nodiscard]] static D3D12MA::Allocator* GetAllocator();

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
    static bool mbInitialized;

    static FD3D12Backend* mpBackend;

    static uint32_t mCurrentFrameIndex;
    static uint64_t mFrameNumber;

    static uint32_t mRenderWidth;
    static uint32_t mRenderHeight;

    static FRenderStats mStats;
};

#define RENDERER_DEVICE()    FRendererCore::GetDevice()
#define RENDERER_BACKEND()   FRendererCore::GetBackend()
#define RENDERER_ALLOCATOR() FRendererCore::GetAllocator()
#define RENDERER_FRAME()     FRendererCore::GetFrameIndex()