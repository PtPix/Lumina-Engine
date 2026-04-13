#pragma once

#include <d3d12.h>
#include <wrl/client.h>

class GpuResource
{
public:
    GpuResource() = default;
    virtual ~GpuResource() = default;

    [[nodiscard]] ID3D12Resource* GetResource() const { return mpResource.Get(); }
    [[nodiscard]] D3D12_RESOURCE_STATES GetUsageState() const { return mUsageState; }
    void SetUsageState(D3D12_RESOURCE_STATES UsageState) { mUsageState = UsageState; }

protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> mpResource;
    D3D12_RESOURCE_STATES mUsageState = D3D12_RESOURCE_STATE_COMMON;
};