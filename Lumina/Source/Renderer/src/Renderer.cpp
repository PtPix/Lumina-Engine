#include "Renderer/Renderer.h"

#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"
#include "Renderer/D3D12Core/Resource/FResourceUploader.h"
#include "Renderer/Managers/FTextureManager.h"

FRootSignature Renderer::mBindlessRootSignature;
FResourceUploader Renderer::mUploader;

bool Renderer::Initialize(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    D3D12Backend::Initialize(Hwnd, Width, Height);

    mUploader.Initialize(D3D12Backend::GetDevice());

    mUploader.BeginUpload();
    TextureManager::Initialize(D3D12Backend::GetDevice(), &mUploader);
    mUploader.EndUpLoadAndExecute();
    mUploader.FlushAndSync();

    InitializeBindlessRootSignature();

    return true;
}

void Renderer::Shutdown()
{
    mUploader.FlushAndSync();
    TextureManager::Shutdown();
    D3D12Backend::Shutdown();
}

FCommandContext* Renderer::BeginFrame()
{
    D3D12Backend::BeginFrame();

    mUploader.CleanUpStaleUploads();

    FCommandContext* pContext = D3D12Backend::AllocateContext();

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    pContext->FlushResourceBarriers();

    return pContext;
}

void Renderer::EndFrame(FCommandContext* pContext)
{
    if (!pContext) return;

    pContext->TransitionResource(D3D12Backend::GetCurrentBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT);
    pContext->FlushResourceBarriers();

    D3D12Backend::ExecuteGraphicsContext(pContext);

    D3D12Backend::EndFrameAndPresent();
}

FMesh* Renderer::CreateMesh(const FMeshData& CpuData)
{
    auto* pMesh = new FMesh();

    mUploader.BeginUpload();
    pMesh->Initialize(CpuData, D3D12Backend::GetAllocator(), &mUploader);
    // TODO : Streaming
    mUploader.FlushAndSync();

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
    std::vector<D3D12_DESCRIPTOR_RANGE1> BindlessRanges;
    D3D12_DESCRIPTOR_RANGE1 SrvRange = {};
    SrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    SrvRange.NumDescriptors = UINT_MAX;
    SrvRange.BaseShaderRegister = 0;
    SrvRange.RegisterSpace = 1;
    SrvRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
    SrvRange.OffsetInDescriptorsFromTableStart = 0;
    BindlessRanges.push_back(SrvRange);


    D3D12_DESCRIPTOR_RANGE1 BufRange = {};
    BufRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    BufRange.NumDescriptors = UINT_MAX;
    BufRange.BaseShaderRegister = 0;
    BufRange.RegisterSpace = 2;
    BufRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
    BufRange.OffsetInDescriptorsFromTableStart = 0;
    BindlessRanges.push_back(BufRange);

    Builder.AddDescriptorTable(BindlessRanges, D3D12_SHADER_VISIBILITY_ALL);
    // Parameter 2 : Global Static Data
    // register(b1, space0)
    // Root CBV
    Builder.AddConstantBufferView(1, 0);
    Builder.AddShaderResourceView(0, 0);
    Builder.AddShaderResourceView(1, 0);

    // Static Sampler
    // register(s0, space0)
    Builder.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC);

    Builder.AllowInputLayout();

    Builder.Build(D3D12Backend::GetDevice()->GetDevice(), mBindlessRootSignature);
}
