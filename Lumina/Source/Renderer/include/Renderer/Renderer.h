/**
 * @file Renderer.h
 * @brief Top-level static Renderer engine class.
 *
 * Manages the global rendering lifecycle, frame resources, render graph,
 * and interactions with the D3D12 Backend.
 */

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <memory>
#include <vector>

#include "Renderer/Core/RendererCore.h"
#include "D3D12/D3D12Common.h"
#include "Renderer/Pipeline/GlobalRootSignature.h"
#include "D3D12/D3D12RootSignature.h"
#include "D3D12/D3D12ResourceUploader.h"
#include "RenderGraph/RenderGraph.h"
#include "Resources/FFrameResource.h"
#include "Resources/FMesh.h"

struct FSceneView;
class FD3D12CommandContext;
class FD3D12Backend;

class Renderer
{
public:
    // ------------------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------------------
    static bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    static void Shutdown();

    static void OnResize(uint32_t Width, uint32_t Height);

    // ------------------------------------------------------------------------
    // Frame Management
    // ------------------------------------------------------------------------
    static void BeginFrame();
    static void EndFrame();

    static void BindGlobalResources(FD3D12CommandContext* pCommandContext);

    // ------------------------------------------------------------------------
    // Resource Management
    // ------------------------------------------------------------------------
    static FMesh* CreateMesh(const FMeshData& CpuData);

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    static FD3D12ResourceUploader* GetUploader() { return &mUploader; }
    static FD3D12Backend* GetD3D12Backend() { return mpD3D12Backend.get(); }
    static FRenderGraph& GetRenderGraph() { return mRenderGraph; }
    static FRGTextureHandle GetBackBufferHandle() { return mBackBufferHandle; }

private:
    static void InitializeBindlessRootSignature();

    static std::unique_ptr<FD3D12Backend> mpD3D12Backend;

    static FD3D12ResourceUploader mUploader;
    static FRenderGraph mRenderGraph;

    static FD3D12CommandContext* mpCurrentFrameContext;

    static FRGTextureHandle mBackBufferHandle;

    static const int NUM_FRAMES = NUM_SWAPCHAIN_BACKBUFFER;

    friend class FRendererCore;
};