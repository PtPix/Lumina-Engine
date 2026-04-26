#pragma once

#include "Material.h"

class SkyboxMaterial : public MaterialBase
{
public:
    SkyboxMaterial() { mRenderPassFlags = ERenderPass::Skybox; }

    bool Initialize(FDevice* Device, RootSignature* RootSig) override;
    void Bind(FCommandContext* Context) const override;
    void Destroy() override;
};
