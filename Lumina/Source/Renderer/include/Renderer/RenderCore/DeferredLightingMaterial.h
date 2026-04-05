#pragma once

#include "Material.h"

class DeferredLightingMaterial : public MaterialBase
{
public:
    DeferredLightingMaterial() { mRenderPassFlags = ERenderPass::DeferredLighting; }

    bool Initialize(ID3D12Device* Device, RootSignature* RootSig) override;
    void Bind(ID3D12GraphicsCommandList* CommandList) const override;
    void Destroy() override;
};
