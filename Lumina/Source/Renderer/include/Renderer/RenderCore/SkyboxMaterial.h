#pragma once

#include "Material.h"

class SkyboxMaterial : public MaterialBase
{
public:
    SkyboxMaterial() { mRenderPassFlags = ERenderPass::Skybox; }

    bool Initialize(ID3D12Device* Device, RootSignature* RootSig) override;
    void Bind(ID3D12GraphicsCommandList* CommandList) const override;
    void Destroy() override;
};
