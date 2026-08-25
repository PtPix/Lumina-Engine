#include "Renderer/RenderGraph/RenderGraph.h"

#include <cassert>

#include "../../include/Renderer/D3D12/D3D12CommandContext.h"

static std::wstring ToWide(const std::string& Text)
{
    return std::wstring(Text.begin(), Text.end());
}

D3D12GpuResource* FRenderGraphContext::GetResource(FRGTextureHandle Handle) const
{
    return mpGraph->GetGpuResource(Handle);
}

FD3D12Texture* FRenderGraphContext::GetTexture(FRGTextureHandle Handle) const
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

uint32_t FRenderGraphContext::GetSRVIndex(FRGTextureHandle Handle) const
{
    return mpGraph->GetSRVIndexInternal(Handle);
}

uint32_t FRenderGraphContext::GetUAVIndex(FRGTextureHandle Handle, uint32_t Mip) const
{
    return mpGraph->GetUAVIndexInternal(Handle, Mip);
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

void FRenderGraph::Initialize(FD3D12Device* pDevice, D3D12MA::Allocator* pAllocator)
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

void FRenderGraph::ClearImportedResources()
{
    for (FResource& Resource : mResources)
    {
        if (Resource.bImported)
        {
            Resource.pImportedResource = nullptr;
            Resource.ImportedRTV = {};
        }
    }
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

FRGTextureHandle FRenderGraph::ImportBackBuffer(const char* Name, D3D12GpuResource* pResource,
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

void FRenderGraph::Execute(FD3D12CommandContext* pCommandContext)
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

D3D12GpuResource* FRenderGraph::GetGpuResource(FRGTextureHandle Handle) const
{
    const FResource& Resource = GetResourceInternal(Handle);
    return Resource.bImported ? Resource.pImportedResource : const_cast<FD3D12Texture*>(&Resource.Texture);
}

bool FRenderGraph::CreatePhysicalTexture(FResource& Resource)
{
    FD3D12TextureDesc TexDesc = {};
    TexDesc.Dimension        = ED3D12TextureDimension::Texture2D;
    TexDesc.Width            = Resource.Desc.Width;
    TexDesc.Height           = Resource.Desc.Height;
    TexDesc.DepthOrArraySize = 1;
    TexDesc.MipLevels        = 1;
    TexDesc.Format           = Resource.Desc.Format;
    TexDesc.InitialState     = D3D12_RESOURCE_STATE_COMMON;
    TexDesc.DebugName        = ToWide(Resource.Name);

    if (HasUsage(Resource.Desc.Usage, ERGTextureUsage::RenderTarget))
        TexDesc.Flags |= ED3D12TextureFlags::AllowRenderTarget;
    if (HasUsage(Resource.Desc.Usage, ERGTextureUsage::DepthStencil))
        TexDesc.Flags |= ED3D12TextureFlags::AllowDepthStencil;
    if (HasUsage(Resource.Desc.Usage, ERGTextureUsage::UnorderedAccess))
        TexDesc.Flags |= ED3D12TextureFlags::AllowUnorderedAccess;
    if (!HasUsage(Resource.Desc.Usage, ERGTextureUsage::ShaderResource) &&
          !HasUsage(Resource.Desc.Usage, ERGTextureUsage::DepthStencil))
    {
        TexDesc.Flags |= ED3D12TextureFlags::DenyShaderResource;
    }

    TexDesc.bUseClearValue = Resource.Desc.bUseClearValue;
    TexDesc.ClearValue     = Resource.Desc.ClearValue;

    return Resource.Texture.Create(mpDevice, mpAllocator, TexDesc);
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
    if (Resource.bImported) return D3D12_CPU_DESCRIPTOR_HANDLE{};
    return Resource.Texture.GetSRV();
}

uint32_t FRenderGraph::GetSRVIndexInternal(FRGTextureHandle Handle) const
{
    const FResource& Resource = GetResourceInternal(Handle);
    if (Resource.bImported) return FD3D12Texture::InvalidBindlessIndex;
    return Resource.Texture.GetBindlessSRVIndex();
}

uint32_t FRenderGraph::GetUAVIndexInternal(FRGTextureHandle Handle, uint32_t Mip) const
{
    const FResource& Resource = GetResourceInternal(Handle);
    if (Resource.bImported) return FD3D12Texture::InvalidBindlessIndex;
    return Resource.Texture.GetBindlessUAVIndex(Mip);
}
