/**
 * @file D3D12GpuResource.h
 * @brief Base class for all GPU resources.
 *
 * Encapsulates the ID3D12Resource COM pointer and tracks the current usage state
 * to facilitate D3D12 resource barrier transitions.
 */

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <cstdint>

class FD3D12GpuResource
{
public:
    FD3D12GpuResource() = default;
    virtual ~FD3D12GpuResource() = default;

    FD3D12GpuResource(const FD3D12GpuResource&) = delete;
    FD3D12GpuResource& operator=(const FD3D12GpuResource&) = delete;

    FD3D12GpuResource(FD3D12GpuResource&&) = default;
    FD3D12GpuResource& operator=(FD3D12GpuResource&&) = default;

    [[nodiscard]] ID3D12Resource* GetResource() const { return mpResource.Get(); }

    // SubResources Support
    void InitStateTracking(uint32_t NumSubresources, D3D12_RESOURCE_STATES InitialState)
    {
        mNumSubresources = (NumSubresources == 0) ? 1 : NumSubresources;
        mAllSubresourcesState = InitialState;
        mbAllSubresourcesSame = true;
        mSubresourceStates.clear();
    }

    [[nodiscard]] uint32_t GetNumSubresources() const { return mNumSubresources; }
    [[nodiscard]] bool AreAllSubresourcesSame() const { return mbAllSubresourcesSame; }
    [[nodiscard]] D3D12_RESOURCE_STATES GetUsageState() const { return mAllSubresourcesState; }

    void SetUsageState(D3D12_RESOURCE_STATES UsageState)
    {
        mAllSubresourcesState = UsageState;
        mbAllSubresourcesSame = true;
        mSubresourceStates.clear();
    }

    [[nodiscard]] D3D12_RESOURCE_STATES GetSubresourceState(uint32_t Subresource) const
    {
        if (mbAllSubresourcesSame) return mAllSubresourcesState;
        return (Subresource < mSubresourceStates.size()) ? mSubresourceStates[Subresource] : mAllSubresourcesState;
    }

    void SetSubresourceState(uint32_t Subresource, D3D12_RESOURCE_STATES NewState)
    {
        SplitSubresourceStates();
        if (Subresource < mSubresourceStates.size())
        {
            mSubresourceStates[Subresource] = NewState;
            TryMergeSubresourceStates();
        }
    }

protected:
    void SplitSubresourceStates()
    {
        if (!mbAllSubresourcesSame) return;

        mSubresourceStates.assign(mNumSubresources, mAllSubresourcesState);
        mbAllSubresourcesSame = false;
    }

    void TryMergeSubresourceStates()
    {
        if (mbAllSubresourcesSame || mSubresourceStates.empty()) return;

        const D3D12_RESOURCE_STATES First = mSubresourceStates[0];
        for (D3D12_RESOURCE_STATES State : mSubresourceStates)
        {
            if (State != First) return;
        }

        mAllSubresourcesState = First;
        mbAllSubresourcesSame = true;
        mSubresourceStates.clear();
    }

protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> mpResource;

    D3D12_RESOURCE_STATES mAllSubresourcesState = D3D12_RESOURCE_STATE_COMMON;
    bool mbAllSubresourcesSame = true;
    uint32_t mNumSubresources = 1;
    std::vector<D3D12_RESOURCE_STATES> mSubresourceStates;

    friend class FD3D12CommandContext;
};