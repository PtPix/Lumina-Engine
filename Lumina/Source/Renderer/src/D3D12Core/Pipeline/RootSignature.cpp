#include "../../../include/Renderer/D3D12Core/Pipeline/RootSignature.h"

FRootSignature::~FRootSignature()
{
    Destroy();
}

void FRootSignature::Destroy()
{
    if (mRootSignature)
    {
        mRootSignature->Release();
        mRootSignature = nullptr;
    }
}

RootSignatureBuilder::RootSignatureBuilder()
{
    mRootParameters.reserve(16);
    mStaticSamplers.reserve(16);
    mDescriptorRangesArray.reserve(16);
}

RootSignatureBuilder& RootSignatureBuilder::AddRootConstants(UINT ShaderRegister, UINT RegisterSpace, UINT NumValues)
{
    D3D12_ROOT_PARAMETER1 RootParameter = {};
    RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    RootParameter.Constants.ShaderRegister = ShaderRegister;
    RootParameter.Constants.RegisterSpace = RegisterSpace;
    RootParameter.Constants.Num32BitValues = NumValues;
    RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    mRootParameters.push_back(RootParameter);
    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddConstantBufferView(UINT ShaderRegister, UINT RegisterSpace)
{
    D3D12_ROOT_PARAMETER1 RootParameter = {};
    RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    RootParameter.Descriptor.ShaderRegister = ShaderRegister;
    RootParameter.Descriptor.RegisterSpace = RegisterSpace;
    RootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
    RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    mRootParameters.push_back(RootParameter);
    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddShaderResourceView(UINT ShaderRegister, UINT RegisterSpace)
{
    D3D12_ROOT_PARAMETER1 RootParameter = {};
    RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    RootParameter.Descriptor.ShaderRegister = ShaderRegister;
    RootParameter.Descriptor.RegisterSpace = RegisterSpace;
    RootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
    RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    mRootParameters.push_back(RootParameter);
    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE1>& Ranges,
    D3D12_SHADER_VISIBILITY Visibility)
{
    mDescriptorRangesArray.push_back(Ranges);

    D3D12_ROOT_PARAMETER1 RootParameter = {};
    RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    RootParameter.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(Ranges.size());
    RootParameter.DescriptorTable.pDescriptorRanges = nullptr;
    RootParameter.ShaderVisibility = Visibility;

    mRootParameters.push_back(RootParameter);
    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddStaticSampler(UINT ShaderRegister, UINT RegisterSpace,
    D3D12_FILTER Filter)
{
    D3D12_STATIC_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = Filter;
    SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerDesc.MipLODBias = 0;
    SamplerDesc.MaxAnisotropy = 0;
    SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    SamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    SamplerDesc.MinLOD = 0.0f;
    SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    SamplerDesc.ShaderRegister = ShaderRegister;
    SamplerDesc.RegisterSpace = RegisterSpace;
    SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    mStaticSamplers.push_back(SamplerDesc);
    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AllowInputLayout()
{
    mFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    return *this;
}

bool RootSignatureBuilder::Build(ID3D12Device* Device, FRootSignature& OutRootSignature)
{
    size_t TableIndex = 0;
    for (auto& RootParam : mRootParameters)
    {
        if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            RootParam.DescriptorTable.pDescriptorRanges = mDescriptorRangesArray[TableIndex].data();
            TableIndex++;
        }
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc = {};
    RootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    RootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(mRootParameters.size());
    RootSignatureDesc.Desc_1_1.pParameters = mRootParameters.empty() ? nullptr : mRootParameters.data();
    RootSignatureDesc.Desc_1_1.NumStaticSamplers = static_cast<UINT>(mStaticSamplers.size());
    RootSignatureDesc.Desc_1_1.pStaticSamplers = mStaticSamplers.data();
    RootSignatureDesc.Desc_1_1.Flags = mFlags;

    D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};
    FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData))))
    {
        FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    ID3DBlob* SignatureBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    HRESULT HResult = D3D12SerializeVersionedRootSignature(&RootSignatureDesc, &SignatureBlob, &ErrorBlob);
    if (FAILED(HResult))
    {
        if (ErrorBlob)
        {
            Log::Error("Failed to serialize root signature: %s", static_cast<char*>(ErrorBlob->GetBufferPointer()));
            ErrorBlob->Release();
        }
        return false;
    }

    OutRootSignature.Destroy();
    HResult = Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), IID_PPV_ARGS(&OutRootSignature.mRootSignature));

    if (SignatureBlob) SignatureBlob->Release();
    if (ErrorBlob) ErrorBlob->Release();

    return SUCCEEDED(HResult);
}
