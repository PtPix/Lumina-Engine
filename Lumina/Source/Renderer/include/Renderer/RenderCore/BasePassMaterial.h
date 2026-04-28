#pragma once

#include "Material.h"

class BasePassMaterial : public MaterialBase
{
public:
    BasePassMaterial() { mRenderPassFlags = ERenderPass::BasePass; }

    bool Initialize(FDevice* Device, FRootSignature* RootSig) override;
    void Bind(FCommandContext* Context) const override;
    void Destroy() override;
};
