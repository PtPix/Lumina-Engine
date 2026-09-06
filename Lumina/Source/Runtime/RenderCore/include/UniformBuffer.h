/**
  * @file UniformBuffer.h
  * @brief Template-based uniform buffer system.
  *
  * Provides a type-safe, multi-frame uniform buffer wrapper.
  * Automatically handles frame indexing and upload.
  */

#pragma once

#include "D3D12Buffer.h"
#include "RenderTypes.h"
#include "RenderCore.h"
#include <cstring>

template <typename TData>
class TUniformBuffer
{
public:
    TUniformBuffer() = default;
    ~TUniformBuffer() { Destroy(); }

    TUniformBuffer(const TUniformBuffer&) = delete;
    TUniformBuffer& operator=(const TUniformBuffer&) = delete;

    // LifeCycle
    bool Create(const wchar_t* DebugName = L"UniformBuffer")
    {
        auto* Allocator = RENDERER_ALLOCATOR();
        if (!Allocator) return false;

        for (uint32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        {
            wchar_t NameBuffer[256];
            swprintf_s(NameBuffer, L"%s_Frame%u", DebugName, i);

            if (!mBuffers[i].Create(Allocator, sizeof(TData), NameBuffer))
            {
                Destroy();
                return false;
            }
        }

        mbInitialized = true;
        return true;
    }

    void Destroy()
    {
        if (!mbInitialized) return;

        for (uint32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        {
            mBuffers[i].Destroy();
        }

        mbInitialized = false;
    }

    // Update
    void Update(const TData& Data, uint32_t FrameIndex)
    {
        if (!mbInitialized || FrameIndex >= NUM_FRAMES_IN_FLIGHT) return;

        void* MappedData = mBuffers[FrameIndex].Map();
        if (MappedData)
        {
            memcpy(MappedData, &Data, sizeof(TData));
            mBuffers[FrameIndex].Unmap();
        }
    }

    void UpdateCurrent(const TData& Data)
    {
        Update(Data, RENDERER_FRAME());
    }

    // Access
    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(uint32_t FrameIndex) const
    {
        if (!mbInitialized || FrameIndex >= NUM_FRAMES_IN_FLIGHT)
        {
            return 0;
        }
        return mBuffers[FrameIndex].GetGPUVirtualAddress();
    }

    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetCurrentGPUAddress() const
    {
        return GetGPUAddress(RENDERER_FRAME());
    }

    [[nodiscard]] FD3D12ConstantBuffer* GetBuffer(uint32_t FrameIndex)
    {
        if (!mbInitialized || FrameIndex >= NUM_FRAMES_IN_FLIGHT)
        {
            return nullptr;
        }
        return &mBuffers[FrameIndex];
    }

    [[nodiscard]] bool IsInitialized() const { return mbInitialized; }

private:
    FD3D12ConstantBuffer mBuffers[NUM_FRAMES_IN_FLIGHT];
    bool mbInitialized = false;
};
