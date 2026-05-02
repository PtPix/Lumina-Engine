#pragma once

class FCommandContext;
struct FSceneView;

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;

    virtual void Initialize() = 0;
    virtual void Execute(FCommandContext* pCommandContext, const FSceneView& View) = 0;
    virtual void Shutdown() = 0;
};