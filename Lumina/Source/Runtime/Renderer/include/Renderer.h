/**
 * @file Renderer.h
 * @brief DEPRECATED: Compatibility wrapper for Renderer class.
 *
 * This class is deprecated. All functionality has been moved to FRendererCore.
 * This file provides compatibility aliases to ease the transition.
 *
 * Please use FRendererCore directly instead of Renderer.
 */

#pragma once

#include "RenderCore.h"
#include "RenderGraph/RenderGraph.h"

// Forward declarations
struct FMeshData;
class FMesh;
class FD3D12Backend;
class FD3D12ResourceUploader;
class FD3D12CommandContext;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/**
 * @brief DEPRECATED: Use FRendererCore instead.
 *
 * This class now forwards all calls to FRendererCore for backward compatibility.
 */
class Renderer
{
public:
    // ------------------------------------------------------------------------
    // Lifecycle - Forward to FRendererCore
    // ------------------------------------------------------------------------
    // Note: Initialize is implemented in Renderer.cpp (not inline)
    // because it needs to initialize Renderer-layer subsystems after RenderCore
    static bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);

    // Note: Shutdown is implemented in Renderer.cpp (not inline)
    // because it needs to shutdown Renderer-layer subsystems before RenderCore
    static void Shutdown();

    static void OnResize(uint32_t Width, uint32_t Height)
    {
        FRendererCore::OnResize(Width, Height);
    }

    // ------------------------------------------------------------------------
    // Frame Management - Forward to FRendererCore
    // ------------------------------------------------------------------------
    static void BeginFrame()
    {
        FRendererCore::BeginFrame();
    }

    static void EndFrame()
    {
        FRendererCore::EndFrame();
    }

    // Note: BindGlobalResources is implemented in Renderer.cpp
    // because it requires GlobalRootSignature from Renderer module
    static void BindGlobalResources(FD3D12CommandContext* pCommandContext);

    // ------------------------------------------------------------------------
    // Resource Management - Forward to FRendererCore
    // ------------------------------------------------------------------------
    // Note: CreateMesh is implemented in Renderer.cpp (not inline)
    // because it requires FMesh definition from Renderer module
    static FMesh* CreateMesh(const FMeshData& CpuData);

    // ------------------------------------------------------------------------
    // Getters - Forward to FRendererCore
    // ------------------------------------------------------------------------
    static FD3D12ResourceUploader* GetUploader()
    {
        return FRendererCore::GetUploader();
    }

    static FD3D12Backend* GetD3D12Backend()
    {
        return FRendererCore::GetBackend();
    }

    static FRenderGraph& GetRenderGraph()
    {
        return *FRendererCore::GetRenderGraph();
    }

    static FRGTextureHandle GetBackBufferHandle()
    {
        return FRendererCore::GetBackBufferHandle();
    }
};