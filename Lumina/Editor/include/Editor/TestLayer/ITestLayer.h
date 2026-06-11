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
    virtual void OnRender(FCommandContext* pCommandContext) {}
    virtual void OnRenderUI() {}
    virtual void OnBuildRenderGraph(FRenderGraph& Graph) {}

    [[nodiscard]] virtual std::string GetName() const = 0;
};