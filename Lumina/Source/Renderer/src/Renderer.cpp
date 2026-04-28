#include "Renderer/Renderer.h"

#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Resource/ResourceUploader.h"

FRootSignature Renderer::mBindlessRootSignature;

bool Renderer::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    D3D12Backend::Initialize(Hwnd, Width, Height);
    InitializeBindlessRootSignature();

    return true;
}

void Renderer::Shutdown()
{
    D3D12Backend::Shutdown();
}

FCommandContext* Renderer::BeginFrame()
{
    D3D12Backend::BeginFrame();

    FCommandContext* pContext = D3D12Backend::AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    pContext->FlushResourceBarriers();

    return pContext;
}

void Renderer::EndFrame(FCommandContext* pContext)
{
    if (!pContext) return;

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT);
    pContext->Close();

    D3D12Backend::GetGraphicsQueue()->ExecuteCommandList(pContext->GetCommandList());

    D3D12Backend::FreeContext(pContext);

    D3D12Backend::EndFrameAndPresent();
    D3D12Backend::FlushGPU();
}

FMesh* Renderer::CreateMesh(const FMeshData& CpuData)
{
    FCommandContext* pCopyContext = D3D12Backend::AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
    FCommandQueue* pCopyQueue = D3D12Backend::GetGraphicsQueue();
    D3D12MA::Allocator* pAllocator = D3D12Backend::GetAllocator();

    ResourceUploader Uploader;
    Uploader.Initialize(pAllocator, pCopyContext, pCopyQueue);

    FMesh* pMesh = new FMesh();
    pMesh->Initialize(CpuData, pAllocator, &Uploader);

    Uploader.FlushAndSync();

    D3D12Backend::FreeContext(pCopyContext);

    return pMesh;
}

void Renderer::InitializeBindlessRootSignature()
{
    RootSignatureBuilder Builder;

    // Parameter 0 : Per Object Bindless Index
    // register(b0, space0)
    // Refers to the object data in descriptor heap
    Builder.AddRootConstants(0, 0, 1);

    // Parameter 1 : Bindless Resource
    // register(t0, space1)
    // Possess all of the SRV, CBV, UAV
    Builder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT_MAX, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

    // Parameter 2 : Global Static Data
    // register(b1, space0)
    // Root CBV
    Builder.AddConstantBufferView(1, 0);

    // Static Sampler
    // register(s0, space0)
    Builder.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC);

    Builder.AllowInputLayout();

    Builder.Build(D3D12Backend::GetDevice()->GetDevice(), mBindlessRootSignature);
}
