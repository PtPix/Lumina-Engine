#pragma once
#include <string>

class GraphicsDevice;
class FCommandContext;

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

    [[nodiscard]] virtual std::string GetName() const = 0;
};