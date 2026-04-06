#pragma once

#include <d3d12.h>
#include <vector>


enum class ERootParameterType
{
    CBV,
    DescriptorTable
};

struct FRootParameterConfig
{
    ERootParameterType Type;
    UINT ShaderRegister;
    UINT RegisterSpace;

    D3D12_DESCRIPTOR_RANGE_TYPE RangeType;
    UINT NumDescriptors;
};

class RootSignature
{
public:
    RootSignature() = default;
    ~RootSignature();

    void Destroy();
    [[nodiscard]] ID3D12RootSignature* Get() const { return mRootSignature; }

private:
    friend class RootSignatureBuilder;
    ID3D12RootSignature* mRootSignature = nullptr;
};

class RootSignatureBuilder
{
public:
    RootSignatureBuilder();

    RootSignatureBuilder& AddRootConstants(UINT ShaderRegister, UINT RegisterSpace, UINT NumValues);
    RootSignatureBuilder& AddConstantBufferView(UINT ShaderRegister, UINT RegisterSpace = 0);
    RootSignatureBuilder& AddShaderResourceView(UINT ShaderRegister, UINT RegisterSpace = 0);
    RootSignatureBuilder& AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE RangeType, UINT NumDescriptors, UINT ShaderRegister, UINT RegisterSpace = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
    RootSignatureBuilder& AddStaticSampler(UINT ShaderRegister, UINT RegisterSpace = 0, D3D12_FILTER Filter = D3D12_FILTER_MIN_MAG_MIP_POINT);

    RootSignatureBuilder& AllowInputLayout();

    bool Build(ID3D12Device* Device, RootSignature& OutRootSignature);

private:
    std::vector<D3D12_ROOT_PARAMETER> mRootParameters;
    D3D12_ROOT_SIGNATURE_FLAGS mFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    std::vector<D3D12_DESCRIPTOR_RANGE> mDescriptorRanges;
    std::vector<D3D12_STATIC_SAMPLER_DESC> mStaticSamplers;
};