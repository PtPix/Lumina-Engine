#include "Renderer/RenderGraph/RenderGraph.h"

#include <cassert>

#include "Renderer/D3D12Core/Core/FCommandContext.h"

static std::wstring ToWide(const std::string& Text)
{
    return std::wstring(Text.begin(), Text.end());
}

GpuResource* FRenderGraphContext::GetResource(FRGTextureHandle Handle) const
{
    return mpGraph->GetGpuResource(Handle);
}

FTexture* FRenderGraphContext::GetTexture(FRGTextureHandle Handle) const
{
    FRenderGraph::FResource& Resource = mpGraph->GetResourceInternal(Handle);
    return Resource.bImported ? nullptr : &Resource.Texture;
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderGraphContext::GetRTV(FRGTextureHandle Handle) const
{
    return mpGraph->GetRTVInternal(Handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderGraphContext::GetDSV(FRGTextureHandle Handle) const
{
    return mpGraph->GetDSVInternal(Handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderGraphContext::GetSRV(FRGTextureHandle Handle) const
{
    return mpGraph->GetSRVInternal(Handle);
}

FRenderGraphPassBuilder& FRenderGraphPassBuilder::ReadTexture(FRGTextureHandle Texture)
{
    mpGraph->mPasses[mPassIndex].Reads.push_back({ Texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE });
    return *this;
}

FRenderGraphPassBuilder& FRenderGraphPassBuilder::WriteRenderTarget(FRGTextureHandle Texture, ERGLoadOp LoadOp)
{
    mpGraph->mPasses[mPassIndex].RenderTargets.push_back({ Texture, LoadOp });
    return *this;
}

FRenderGraphPassBuilder& FRenderGraphPassBuilder::WriteDepth(FRGTextureHandle Texture, ERGLoadOp LoadOp)
{
    mpGraph->mPasses[mPassIndex].bHasDepth = true;
    mpGraph->mPasses[mPassIndex].Depth = { Texture, LoadOp };
    return *this;
}

FRenderGraphPassBuilder& FRenderGraphPassBuilder::ReadWriteUAV(FRGTextureHandle Texture)
{
    mpGraph->mPasses[mPassIndex].UAVs.push_back({ Texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS });
    return *this;
}

FRenderGraphPassBuilder& FRenderGraphPassBuilder::Execute(std::function<void(FRenderGraphContext&)> ExecuteFunc)
{
    mpGraph->mPasses[mPassIndex].ExecuteFunc = std::move(ExecuteFunc);
    return *this;
}

void FRenderGraph::Initialize(FDevice* pDevice, D3D12MA::Allocator* pAllocator)
{
    mpDevice = pDevice;
    mpAllocator = pAllocator;
}

void FRenderGraph::Shutdown()
{
    for (FResource& Resource : mResources)
    {
        if (!Resource.bImported) Resource.Texture.Destroy();
    }

    mResources.clear();
    mResourceNameToIndex.clear();
    mPasses.clear();

    mpAllocator = nullptr;
    mpDevice = nullptr;
}

void FRenderGraph::Reset()
{
    mPasses.clear();
}

FRGTextureHandle FRenderGraph::CreateTexture(const char* Name, const FRGTextureDesc& Desc)
{
    assert(Name != nullptr);
    assert(mpAllocator != nullptr);
    assert(mpDevice != nullptr);

    auto It = mResourceNameToIndex.find(Name);
    // If we find an existing texture
    if (It != mResourceNameToIndex.end())
    {
        FResource& Existing = mResources[It->second];
        if (!Existing.bImported && IsSameDesc(Existing.Desc, Desc))
            return { It->second };

        if (!Existing.bImported)
            Existing.Texture.Destroy();

        Existing.Name = Name;
        Existing.Desc = Desc;
        Existing.bImported = false;
        Existing.pImportedResource = nullptr;
        Existing.ImportedRTV = {};

        CreatePhysicalTexture(Existing);
        return { It->second };
    }

    // We don't find an existing texture, create a new one
    FResource Resource;
    Resource.Name = Name;
    Resource.Desc = Desc;
    Resource.bImported = false;

    CreatePhysicalTexture(Resource);

    uint32_t Index = static_cast<uint32_t>(mResources.size());
    mResources.push_back(std::move(Resource));
    mResourceNameToIndex[Name] = Index;

    return { Index };
}

FRGTextureHandle FRenderGraph::GetTexture(const char* Name)
{
    assert(Name != nullptr);
    assert(mpAllocator != nullptr);
    assert(mpDevice != nullptr);

    auto It = mResourceNameToIndex.find(Name);
    if (It != mResourceNameToIndex.end())
    {
        return { It->second };
    }

    return { UINT32_MAX };
}

FRGTextureHandle FRenderGraph::ImportBackBuffer(const char* Name, GpuResource* pResource,
                                                D3D12_CPU_DESCRIPTOR_HANDLE Rtv, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
                                                D3D12_RESOURCE_STATES InitialState)
{
    assert(Name != nullptr);
    assert(pResource != nullptr);

    pResource->SetUsageState(InitialState);

    FRGTextureDesc Desc = {};
    Desc.Width = Width;
    Desc.Height = Height;
    Desc.Format = Format;
    Desc.Usage = ERGTextureUsage::RenderTarget | ERGTextureUsage::Present;

    auto It = mResourceNameToIndex.find(Name);
    if (It != mResourceNameToIndex.end())
    {
        FResource& Existing = mResources[It->second];
        Existing.Name = Name;
        Existing.Desc = Desc;
        Existing.bImported = true;
        Existing.pImportedResource = pResource;
        Existing.ImportedRTV = Rtv;
        return { It->second };
    }

    FResource Resource;
    Resource.Name = Name;
    Resource.Desc = Desc;
    Resource.bImported = true;
    Resource.pImportedResource = pResource;
    Resource.ImportedRTV = Rtv;

    uint32_t Index = static_cast<uint32_t>(mResources.size());
    mResources.push_back(std::move(Resource));
    mResourceNameToIndex[Name] = Index;

    return { Index };
}

FRenderGraphPassBuilder FRenderGraph::AddPass(const char* Name)
{
    FPass Pass;
    Pass.Name = Name ? Name : "Unknown";

    uint32_t Index = static_cast<uint32_t>(mPasses.size());
    mPasses.push_back(std::move(Pass));

    return FRenderGraphPassBuilder(this, Index);
}

void FRenderGraph::Compile()
{
    // TODO: Compile
}

void FRenderGraph::Execute(FCommandContext* pCommandContext)
{
    assert(pCommandContext != nullptr);

    FRenderGraphContext Context;
    Context.mpGraph = this;
    Context.mpCommandContext = pCommandContext;

    for (FPass& Pass : mPasses)
    {
        for (const FTextureAccess& Read : Pass.Reads)
        {
            pCommandContext->TransitionResource(GetGpuResource(Read.Texture), Read.State);
        }

        for (const FTextureAccess& UAV : Pass.UAVs)
        {
            pCommandContext->TransitionResource(GetGpuResource(UAV.Texture), UAV.State);
        }

        for (const FRenderTargetAccess& RT : Pass.RenderTargets)
        {
            pCommandContext->TransitionResource(GetGpuResource(RT.Texture), D3D12_RESOURCE_STATE_RENDER_TARGET);
        }

        if (Pass.bHasDepth)
        {
            pCommandContext->TransitionResource(GetGpuResource(Pass.Depth.Texture), D3D12_RESOURCE_STATE_DEPTH_WRITE);
        }

        pCommandContext->FlushResourceBarriers();

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Rtvs;
        Rtvs.reserve(Pass.RenderTargets.size());
        for (const FRenderTargetAccess& RT : Pass.RenderTargets)
        {
            Rtvs.push_back(GetRTVInternal(RT.Texture));
        }

        D3D12_CPU_DESCRIPTOR_HANDLE Dsv = {};
        D3D12_CPU_DESCRIPTOR_HANDLE* pDsv = nullptr;
        if (Pass.bHasDepth)
        {
            Dsv = GetDSVInternal(Pass.Depth.Texture);
            pDsv = &Dsv;
        }

        if (!Rtvs.empty() || pDsv)
        {
            pCommandContext->SetRenderTargets(static_cast<UINT>(Rtvs.size()),
                Rtvs.empty() ? nullptr : Rtvs.data(), pDsv);
        }

        for (const FRenderTargetAccess& RT : Pass.RenderTargets)
        {
            if (RT.LoadOp == ERGLoadOp::Clear)
            {
                const FResource& Resource = GetResourceInternal(RT.Texture);
                const float* ClearColor = Resource.Desc.bUseClearValue ? Resource.Desc.ClearValue.Color : nullptr;
                const float DefaultClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

                pCommandContext->ClearRenderTargetView(GetRTVInternal(RT.Texture), ClearColor ? ClearColor : DefaultClearColor);
            }
        }

        if (Pass.bHasDepth && Pass.Depth.LoadOp == ERGLoadOp::Clear)
        {
            const FResource& Resource = GetResourceInternal(Pass.Depth.Texture);

            float ClearDepth = 1.0f;
            uint8_t ClearStencil = 0;

            if (Resource.Desc.bUseClearValue)
            {
                ClearDepth = Resource.Desc.ClearValue.DepthStencil.Depth;
                ClearStencil = Resource.Desc.ClearValue.DepthStencil.Stencil;
            }

            pCommandContext->ClearDepthStencilView(GetDSVInternal(Pass.Depth.Texture),
                D3D12_CLEAR_FLAG_DEPTH, ClearDepth, ClearStencil);
        }

        if (Pass.ExecuteFunc)
            Pass.ExecuteFunc(Context);
    }
}

FRenderGraph::FResource& FRenderGraph::GetResourceInternal(FRGTextureHandle Handle)
{
    assert(Handle.IsValid());
    assert(Handle.Index < mResources.size());
    return mResources[Handle.Index];
}

const FRenderGraph::FResource& FRenderGraph::GetResourceInternal(FRGTextureHandle Handle) const
{
    assert(Handle.IsValid());
    assert(Handle.Index < mResources.size());
    return mResources[Handle.Index];
}

GpuResource* FRenderGraph::GetGpuResource(FRGTextureHandle Handle) const
{
    const FResource& Resource = GetResourceInternal(Handle);
    return Resource.bImported ? Resource.pImportedResource : const_cast<FTexture*>(&Resource.Texture);
}

bool FRenderGraph::CreatePhysicalTexture(FResource& Resource)
{
    D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;

    if (HasUsage(Resource.Desc.Usage, ERGTextureUsage::RenderTarget))
    {
        Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    if (HasUsage(Resource.Desc.Usage, ERGTextureUsage::DepthStencil))
    {
        Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }

    if (!HasUsage(Resource.Desc.Usage, ERGTextureUsage::ShaderResource))
    {
        Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }

    const D3D12_CLEAR_VALUE* pClearValue =
        Resource.Desc.bUseClearValue ? &Resource.Desc.ClearValue : nullptr;

    return Resource.Texture.Create(
        mpDevice,
        mpAllocator,
        Resource.Desc.Width,
        Resource.Desc.Height,
        Resource.Desc.Format,
        Flags,
        D3D12_RESOURCE_STATE_COMMON,
        pClearValue,
        ToWide(Resource.Name)
    );
}

bool FRenderGraph::IsSameDesc(const FRGTextureDesc& A, const FRGTextureDesc& B) const
{
    return A.Width == B.Width
        && A.Height == B.Height
        && A.Format == B.Format
        && static_cast<uint32_t>(A.Usage) == static_cast<uint32_t>(B.Usage);
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderGraph::GetRTVInternal(FRGTextureHandle Handle) const
{
    const FResource& Resource = GetResourceInternal(Handle);
    return Resource.bImported ? Resource.ImportedRTV : Resource.Texture.GetRTV();
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderGraph::GetDSVInternal(FRGTextureHandle Handle) const
{
    const FResource& Resource = GetResourceInternal(Handle);
    return Resource.Texture.GetDSV();
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderGraph::GetSRVInternal(FRGTextureHandle Handle) const
{
    const FResource& Resource = GetResourceInternal(Handle);
    return Resource.Texture.GetSRV();
}