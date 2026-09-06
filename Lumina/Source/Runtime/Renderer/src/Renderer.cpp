/**
 * @file Renderer.cpp
 * @brief Implementation of Renderer compatibility layer and Renderer-specific functions.
 *
 * Most functionality has been moved to FRendererCore.
 * This file contains implementations that require Renderer layer types (like FMesh).
 */

#include "Renderer.h"
#include "FMesh.h"
#include "MeshType.h"
#include "RenderCore.h"
#include "D3D12Backend.h"
#include "D3D12CommandContext.h"
#include "D3D12ResourceUploader.h"
#include "D3D12BindlessDescriptorHeap.h"
#include "Shaders/GlobalRootSignature.h"
#include "Resources/TextureManager.h"
#include "Pipeline/PipelineStateCache.h"
#include "Shaders/ShaderManager.h"
#include "Logger/Logger.h"

// Implementation of Initialize (initializes RenderCore + Renderer layer subsystems)
bool Renderer::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    // Initialize RenderCore first
    FRendererInitParams Params;
    Params.WindowHandle = Hwnd;
    Params.Width = Width;
    Params.Height = Height;

    if (!FRendererCore::Initialize(Params))
    {
        return false;
    }

    // Initialize Renderer layer subsystems
    TextureManager::Initialize(FRendererCore::GetDevice(), FRendererCore::GetUploader());

    // Flush initial uploads
    FRendererCore::GetUploader()->FlushAndSync();

    // Initialize GlobalRootSignature
    if (!FGlobalRootSignature::Initialize(FRendererCore::GetDevice()))
    {
        LUMINA_LOG_ERROR(RHI, "Global RootSignature initialization failed.");
    }

    // Initialize PipelineStateCache
    FPipelineStateCache::Initialize(FRendererCore::GetDevice());

    return true;
}

// Implementation of Shutdown (shuts down Renderer layer subsystems + RenderCore)
void Renderer::Shutdown()
{
    // Shutdown Renderer layer subsystems first
    FGlobalRootSignature::Shutdown();
    FPipelineStateCache::Shutdown();
    FShaderManager::Clear();
    TextureManager::Shutdown();

    // Shutdown RenderCore
    FRendererCore::Shutdown();
}

// Implementation of CreateMesh (requires FMesh definition from Renderer module)
FMesh* Renderer::CreateMesh(const FMeshData& CpuData)
{
    auto* pMesh = new FMesh();
    pMesh->Initialize(CpuData, FRendererCore::GetAllocator(), FRendererCore::GetUploader());
    return pMesh;
}

// Implementation of BindGlobalResources (requires GlobalRootSignature from Renderer module)
void Renderer::BindGlobalResources(FD3D12CommandContext* pContext)
{
    if (!pContext) return;

    FD3D12BindlessDescriptorHeap* pHeap = FRendererCore::GetBackend()->GetBindlessDescriptorHeap();
    if (!pHeap) return;

    ID3D12DescriptorHeap* ppHeaps[] = { pHeap->GetDescriptorHeap() };
    pContext->SetDescriptorHeaps(1, ppHeaps);

    // Graphics
    ID3D12RootSignature* pGraphicsRS = FGlobalRootSignature::GetGraphicsRootSignature();
    if (pGraphicsRS)
    {
        pContext->SetGraphicsRootSignature(pGraphicsRS);
        pContext->SetGraphicsRootDescriptorTable(ToRootIndex(EGlobalRootParam::BindlessTable), pHeap->GetGpuHandle(0));
    }

    // Compute
    ID3D12RootSignature* pComputeRS = FGlobalRootSignature::GetComputeRootSignature();
    if (pComputeRS)
    {
        pContext->SetComputeRootSignature(pComputeRS);
        pContext->SetComputeRootDescriptorTable(ToRootIndex(EGlobalRootParam::BindlessTable), pHeap->GetGpuHandle(0));
    }
}
