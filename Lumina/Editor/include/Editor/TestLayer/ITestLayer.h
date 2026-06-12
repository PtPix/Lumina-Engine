#pragma once
#include <string>

class FCommandContext;
class FRenderGraph;

class ITestLayer
{
public:
    virtual ~ITestLayer() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}

    virtual void OnUpdate(double DeltaTime) {}
    virtual void OnFixedUpdate(double FixedDeltaTime) {}
    virtual void OnRender(FRenderGraph& RenderGraph) {}
    virtual void OnRenderUI() {}

    [[nodiscard]] virtual std::string GetName() const = 0;
};