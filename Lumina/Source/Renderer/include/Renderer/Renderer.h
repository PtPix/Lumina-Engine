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

#include "Renderer/D3D12Core/Common.h"
#include "Renderer/D3D12Core/Pipeline/RootSignature.h"
#include "Renderer/D3D12Core/Resource/ResourceUploader.h"
#include "RenderGraph/RenderGraph.h"
#include "Resources/FFrameResource.h"
#include "Resources/FMesh.h"

struct FSceneView;
class FCommandContext;
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

    static void UploadSceneView(const FSceneView& View);
    static void BindGlobalResources(FCommandContext* pCommandContext);

    // ------------------------------------------------------------------------
    // Resource Management
    // ------------------------------------------------------------------------
    static FMesh* CreateMesh(const FMeshData& CpuData);

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    static FRootSignature* GetBindlessRootSignature() { return &mBindlessRootSignature; }
    static FResourceUploader* GetUploader() { return &mUploader; }
    static FD3D12Backend* GetD3D12Backend() { return mpD3D12Backend.get(); }
    static FRenderGraph& GetRenderGraph() { return mRenderGraph; }
    static FRGTextureHandle GetBackBufferHandle() { return mBackBufferHandle; }

    static uint32_t           GetCurrentFrameIndex()     { return mCurrentFrameIndex; }
    static FFrameResource&    GetCurrentFrameResource()  { return mFrameResources[mCurrentFrameIndex]; }

private:
    static void InitializeBindlessRootSignature();
    static void InitializeSceneBuffers();
    static void DestroySceneBuffers();

    static std::unique_ptr<FD3D12Backend> mpD3D12Backend;

    static FRootSignature mBindlessRootSignature;
    static FResourceUploader mUploader;
    static FRenderGraph mRenderGraph;

    static FCommandContext* mpCurrentFrameContext;

    static FRGTextureHandle mBackBufferHandle;

    static const int NUM_FRAMES = NUM_SWAPCHAIN_BACKBUFFER;
    static FFrameResource mFrameResources[NUM_FRAMES];
    static uint32_t mCurrentFrameIndex;
};