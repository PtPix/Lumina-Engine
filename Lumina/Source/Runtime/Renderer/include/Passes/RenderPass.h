/**
  * @file RenderPass.h
  * @brief Base class for all render passes.
  *
  * Provides a standard interface for Graphics, Compute, and RayTracing passes.
  * All passes inherit from this and implement Setup/Execute.
  */

#pragma once

#include "RenderGraph/RenderGraph.h"
#include <string>

class FViewInfo;
class FGPUScene;
struct FMeshDrawCommand;

struct FPassContext
{
    FRenderGraphContext* RGContext = nullptr;
    FViewInfo* ViewInfo = nullptr;
    FGPUScene* GPUScene = nullptr;

    const std::vector<FMeshDrawCommand>* DrawCommands = nullptr;

    [[nodiscard]] FD3D12CommandContext* GetCommandContext() const
    {
        return RGContext ? RGContext->GetCommandContext() : nullptr;
    }
};

class FRenderPassBase
{
public:
    explicit FRenderPassBase(const char* PassName) : mPassName(PassName) {}

    virtual ~FRenderPassBase() = default;

    FRenderPassBase(const FRenderPassBase&) = delete;
    FRenderPassBase& operator=(const FRenderPassBase&) = delete;

    // ------------------------------------------------------------------------
    // Lifecycle (Override in derived classes)
    // ------------------------------------------------------------------------
    virtual void Setup(FRenderGraphPassBuilder& Builder) = 0;
    virtual void Execute(const FPassContext& Context) = 0;

    // ------------------------------------------------------------------------
    // Pass Info
    // ------------------------------------------------------------------------
    [[nodiscard]] const char* GetName() const { return mPassName.c_str(); }

private:
    std::string mPassName;
};

class FGraphicsPassBase : public FRenderPassBase
{
public:
    explicit FGraphicsPassBase(const char* PassName) : FRenderPassBase(PassName) {}

    virtual ~FGraphicsPassBase() = default;

protected:
    void BindRenderTargets(FD3D12CommandContext* CommandContext,
        const D3D12_CPU_DESCRIPTOR_HANDLE* RTVs, uint32_t NumRTVs,
        const D3D12_CPU_DESCRIPTOR_HANDLE* DSV, uint32_t ViewportWidth, uint32_t ViewportHeight);
};

class FComputePassBase : public FRenderPassBase
{
public:
    explicit FComputePassBase(const char* PassName) : FRenderPassBase(PassName) {}

    virtual ~FComputePassBase() = default;

protected:
    void Dispatch(FD3D12CommandContext* CommandContext,
        uint32_t ThreadGroupX, uint32_t ThreadGroupY, uint32_t ThreadGroupZ);
};