/**
  * @file DebugVisualizePass.h
  * @brief Debug visualization pass for depth/normals/etc.
  */

#pragma once

#include "Passes/FullscreenPass.h"
#include "RenderGraph/RenderGraph.h"

enum class EDebugVisualizeMode : uint8_t
{
    Depth,
    Normals,
    Albedo,
    Metallic,
    Roughness,
};

struct FDebugVisualizeInputs
{
    FRGTextureHandle InputTexture;
    FRGTextureHandle OutputTexture;
    EDebugVisualizeMode Mode = EDebugVisualizeMode::Depth;
};

struct FDebugVisualizeOutputs {};

class FDebugVisualizePass : public FFullscreenPassBase
{
public:
    explicit FDebugVisualizePass(const FDebugVisualizeInputs& Inputs);

    void Setup(FRenderGraphPassBuilder &Builder) override;
    void Execute(const FPassContext &Context) override;

    [[nodiscard]] const FDebugVisualizeOutputs& GetOutputs() const { return mOutputs; }

private:
    FDebugVisualizeInputs mInputs;
    FDebugVisualizeOutputs mOutputs;
};

inline FDebugVisualizePass* AddDebugVisualizePass(FRenderGraph& RG, const FDebugVisualizeInputs& Inputs)
{
    return RG.AddPassObject<FDebugVisualizePass>(Inputs);
}