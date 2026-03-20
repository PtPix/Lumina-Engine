#pragma once

#include <cstdint>
#include <windows.h>

struct ID3D12CommandQueue;
struct ID3D12Device;
struct ID3D12Fence;

class Fence
{
public:
    void Create(ID3D12Device* pDevice, const char* pDebugName);
    void Destroy();

    void Signal(ID3D12CommandQueue* pCommandQueue);
    [[nodiscard]] uint64_t GetValue() const { return mFenceValue; }

    void WaitOnCPU(uint64_t FenceWaitValue) const;
    void WaitOnGPU(ID3D12CommandQueue* pCommandQueue);
    void WaitOnGPU(ID3D12CommandQueue* pCommandQueue, uint64_t OlderFence);

private:
    HANDLE       mHEvent     = nullptr;
    ID3D12Fence* mpFence     = nullptr;
    uint64_t     mFenceValue = 0;
};