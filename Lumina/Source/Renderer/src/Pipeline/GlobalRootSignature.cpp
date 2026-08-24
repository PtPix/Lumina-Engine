#include "Renderer/Pipeline/GlobalRootSignature.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Pipeline/RootSignature.h"

FRootSignature FGlobalRootSignature::mGraphicsRootSignature;
FRootSignature FGlobalRootSignature::mComputeRootSignature;
bool FGlobalRootSignature::mbInitialized = false;

bool FGlobalRootSignature::Initialize(FDevice *pDevice)
{
    if (mbInitialized) return true;

    if (!BuildRootSignature(pDevice, mGraphicsRootSignature)) return false;
    if (!BuildRootSignature(pDevice, mComputeRootSignature)) return false;

    mbInitialized = true;
    return true;
}

void FGlobalRootSignature::Shutdown()
{
    mGraphicsRootSignature.Destroy();
    mComputeRootSignature.Destroy();
    mbInitialized = false;
}

ID3D12RootSignature * FGlobalRootSignature::GetGraphicsRootSignature()
{
    return mbInitialized ? mGraphicsRootSignature.Get() : nullptr;
}

ID3D12RootSignature * FGlobalRootSignature::GetComputeRootSignature()
{
    return mbInitialized ? mComputeRootSignature.Get() : nullptr;
}

bool FGlobalRootSignature::BuildRootSignature(FDevice *pDevice, FRootSignature &OutRootSignature)
{
    FRootSignatureBuilder Builder;

    // Slot 0 : Root Constants b0 space1
    Builder.AddRootConstants(0, 1, 16);

    // Slot 1 : View CBV b0 space0
    Builder.AddConstantBufferView(0, 0);

    // Slot 2 : Bindless Table
    std::vector<D3D12_DESCRIPTOR_RANGE1> Ranges;

    // SRV: t0 space1 (Texture)
    D3D12_DESCRIPTOR_RANGE1 SrvRange = {};
    SrvRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    SrvRange.NumDescriptors                    = UINT_MAX;
    SrvRange.BaseShaderRegister                = 0;
    SrvRange.RegisterSpace                     = 1;
    SrvRange.Flags                             = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
    SrvRange.OffsetInDescriptorsFromTableStart = 0;
    Ranges.push_back(SrvRange);

    // SRV: t0 space2 (Structure Buffer / Byte Address Buffer)
    D3D12_DESCRIPTOR_RANGE1 BufRange = {};
    BufRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    BufRange.NumDescriptors                    = UINT_MAX;
    BufRange.BaseShaderRegister                = 0;
    BufRange.RegisterSpace                     = 2;
    BufRange.Flags                             = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
    BufRange.OffsetInDescriptorsFromTableStart = 0;
    Ranges.push_back(BufRange);

    // UAV: u0 space3 (All writable resources)
    D3D12_DESCRIPTOR_RANGE1 UavRange = {};
    UavRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    UavRange.NumDescriptors                    = UINT_MAX;
    UavRange.BaseShaderRegister                = 0;
    UavRange.RegisterSpace                     = 3;
    UavRange.Flags                             = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
    UavRange.OffsetInDescriptorsFromTableStart = 0;
    Ranges.push_back(UavRange);

    // CBV: b1 space0 (per-pass / per-material constant buffer)
    D3D12_DESCRIPTOR_RANGE1 CbvRange = {};
    CbvRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    CbvRange.NumDescriptors                    = UINT_MAX;
    CbvRange.BaseShaderRegister                = 1;
    CbvRange.RegisterSpace                     = 0;
    CbvRange.Flags                             = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
    CbvRange.OffsetInDescriptorsFromTableStart = 0;
    Ranges.push_back(CbvRange);

    Builder.AddDescriptorTable(Ranges, D3D12_SHADER_VISIBILITY_ALL);

    Builder.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC);

    Builder.AllowInputLayout();

    return Builder.Build(pDevice->GetDevice(), OutRootSignature);
}


