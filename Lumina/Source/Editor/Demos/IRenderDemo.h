#pragma once

#include <string>

class FRenderGraph;

class IRenderDemo
{
public:
    virtual ~IRenderDemo() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(double DeltaTime) {}
    virtual void OnRender(FRenderGraph& Graph) {}
    virtual void OnRenderUI() {}

    [[nodiscard]] virtual std::string GetName() const = 0;
    [[nodiscard]] virtual std::string GetDescription() const { return ""; }
};