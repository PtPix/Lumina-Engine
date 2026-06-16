/**
* @file RootSignature.h
 * @brief DirectX 12 Root Signature Wrapper and Builder.
 *
 * Provides a builder interface (RootSignatureBuilder) for defining root constants,
 * descriptor tables, and static samplers required by the rendering pipeline.
 */

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
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

class FRootSignature
{
public:
    FRootSignature() = default;
    ~FRootSignature() { Destroy(); }

    FRootSignature(const FRootSignature&) = delete;
    FRootSignature& operator=(const FRootSignature&) = delete;

    void Destroy();
    [[nodiscard]] ID3D12RootSignature* Get() const { return mRootSignature.Get(); }

private:
    friend class FRootSignatureBuilder;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
};

class FRootSignatureBuilder
{
public:
    FRootSignatureBuilder();

    // ------------------------------------------------------------------------
    // Parameter Definitions
    // ------------------------------------------------------------------------
    FRootSignatureBuilder& AddRootConstants(UINT ShaderRegister, UINT RegisterSpace, UINT NumValues);
    FRootSignatureBuilder& AddConstantBufferView(UINT ShaderRegister, UINT RegisterSpace = 0);
    FRootSignatureBuilder& AddShaderResourceView(UINT ShaderRegister, UINT RegisterSpace = 0);
    FRootSignatureBuilder& AddDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE1>& Ranges, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
    FRootSignatureBuilder& AddStaticSampler(UINT ShaderRegister, UINT RegisterSpace = 0, D3D12_FILTER Filter = D3D12_FILTER_MIN_MAG_MIP_POINT);

    FRootSignatureBuilder& AllowInputLayout();

    // ------------------------------------------------------------------------
    // Build Output
    // ------------------------------------------------------------------------
    bool Build(ID3D12Device* Device, FRootSignature& OutRootSignature);

private:
    std::vector<D3D12_ROOT_PARAMETER1> mRootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> mStaticSamplers;
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> mDescriptorRangesArray;

    D3D12_ROOT_SIGNATURE_FLAGS mFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
};