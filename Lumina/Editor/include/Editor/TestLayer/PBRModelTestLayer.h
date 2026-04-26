#pragma once
#include "ITestLayer.h"

class PBRModelTestLayer : public ITestLayer
{
public:
    std::string GetName() const override { return "PBR Model Rendering"; }

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(double DeltaTime) override;
    void OnRender(FCommandContext* pCommandContext) override;
    void OnRenderUI() override;
};
