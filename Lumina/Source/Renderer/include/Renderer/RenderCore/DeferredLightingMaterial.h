#pragma once

#include "Material.h"

class DeferredLightingMaterial : public MaterialBase
{
public:
    DeferredLightingMaterial() { mRenderPassFlags = ERenderPass::DeferredLighting; }

    bool Initialize(FDevice* Device, RootSignature* RootSig) override;
    void Bind(FCommandContext* Context) const override;
    void Destroy() override;
};
