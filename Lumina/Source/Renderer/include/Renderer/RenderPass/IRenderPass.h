#pragma once

class FCommandContext;
struct FSceneView;
class FDevice;

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;

    virtual void Initialize(FDevice* pDevice) = 0;
    virtual void Execute(FCommandContext* pCommandContext, const FSceneView& View) = 0;
    virtual void Shutdown() = 0;
};