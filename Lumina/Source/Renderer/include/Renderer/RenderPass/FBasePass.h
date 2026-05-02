#pragma once

#include <d3d12.h>
#include <memory>
#include <wrl/client.h>

#include "Renderer/D3D12Core/Pipeline/PipelineState.h"
#include "Renderer/RenderPass/IRenderPass.h"

class FBasePass : public IRenderPass
{
public:
    void Initialize() override;
    void Execute(FCommandContext* pCommandContext, const FSceneView& View) override;
    void Shutdown() override;

private:
    std::unique_ptr<PipelineState> mBasePassPSO;
};