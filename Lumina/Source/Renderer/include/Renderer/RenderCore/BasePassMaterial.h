#pragma once

#include "Material.h"

class BasePassMaterial : public MaterialBase
{
public:
    BasePassMaterial() { mRenderPassFlags = ERenderPass::BasePass; }

    bool Initialize(ID3D12Device* Device, RootSignature* RootSig) override;
    void Bind(ID3D12GraphicsCommandList* CommandList) const override;
    void Destroy() override;
};
