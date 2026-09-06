/**
  * @file BasePass.h
  * @brief Deferred Base pass
  */

#pragma once

#include "Passes/RenderPass.h"
#include "RenderGraph/RenderGraph.h"

struct FBasePassInputs
{
    FRGTextureHandle SceneColor;
};

struct FBasePassOutputs
{
    FRGTextureHandle SceneDepth;
};

class FBasePass : public FGraphicsPassBase
{
public:
    explicit FBasePass(const FBasePassInputs& Inputs);

    void Setup(FRenderGraphPassBuilder& Builder) override;
    void Execute(const FPassContext& Context) override;

    [[nodiscard]] const FBasePassOutputs& GetOutputs() const { return mOutputs; }

private:
    FBasePassInputs mInputs;
    FBasePassOutputs mOutputs;
};

inline FBasePass* AddBasePass(FRenderGraph& RG, const FBasePassInputs& Inputs)
{
    return RG.AddPassObject<FBasePass>(Inputs);
}