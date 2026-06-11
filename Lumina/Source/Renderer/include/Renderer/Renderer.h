#pragma once

#include <windows.h>

#include "D3D12Core/Common.h"
#include "Renderer/D3D12Core/Pipeline/RootSignature.h"
#include "Renderer/D3D12Core/Resource/FResourceUploader.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderPass/FBasePass.h"
#include "Resources/FFrameResource.h"
#include "Resources/FMesh.h"

struct FSceneView;
class FCommandContext;

class Renderer
{
public:
    static bool Initialize(HWND Hwnd, uint32_t Width, uint32_t Height);
    static void Shutdown();

    static FCommandContext* BeginFrame();
    static void EndFrame(FCommandContext* pContext);

    static FMesh* CreateMesh(const FMeshData& CpuData);

    static FRootSignature* GetBindlessRootSignature() { return &mBindlessRootSignature; }

    static FResourceUploader* GetUploader() { return &mUploader; }

    static FD3D12Backend* GetD3D12Backend() { return mpD3D12Backend.get(); }

    static void InitializeSceneBuffers();
    static void DestroySceneBuffers();

    static void RenderSceneView(FCommandContext* pCommandContext, const FSceneView& View);
    static void OnResize(uint32_t Width, uint32_t Height);

    static FRenderGraph& GetRenderGraph() { return mRenderGraph; }
    static FRGTextureHandle GetBackBufferHandle() { return mBackBufferHandle; }

private:
    static void InitializeBindlessRootSignature();
    static std::unique_ptr<FBasePass> mBasePass;
    static FRootSignature mBindlessRootSignature;
    static FResourceUploader mUploader;

    static const int NUM_FRAMES = NUM_SWAPCHAIN_BACKBUFFER;
    static FFrameResource mFrameResources[NUM_FRAMES];
    static uint32_t mCurrentFrameIndex;

    static std::unique_ptr<FD3D12Backend> mpD3D12Backend;

    static FRenderGraph mRenderGraph;
    static FRGTextureHandle mBackBufferHandle;
};