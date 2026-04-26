// #pragma once
//
// #include <d3d12.h>
// #include <wrl/client.h>
//
// #include "D3D12MemAlloc.h"
// #include "../D3D12Core/ResourceHeaps.h"
// #include "Renderer/D3D12Core/ResourceView.h"
//
// struct FRenderTarget
// {
//     Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
//     D3D12MA::Allocation* Allocation = nullptr;
//     ResourceView RtvView;
//     DXGI_FORMAT Format;
//
//     void Release()
//     {
//         pResource.Reset();
//         if (Allocation)
//         {
//             Allocation->Release();
//             Allocation = nullptr;
//         }
//     }
// };
//
// class SceneRenderTargets
// {
// public:
//     SceneRenderTargets() = default;
//     ~SceneRenderTargets();
//
//     bool Initialize(ID3D12Device* pDevice, D3D12MA::Allocator* pAllocator,
//         StaticResourceViewHeap* pRtvHeap, StaticResourceViewHeap* pSrvHeap,
//         StaticResourceViewHeap* pDsvHeap,
//         uint32_t Width, uint32_t Height);
//
//
//     void Resize(uint32_t Width, uint32_t Height);
//
//     void BindBasePass(ID3D12GraphicsCommandList* pCommandList);
//
//     void Clear(ID3D12GraphicsCommandList* pCommandList);
//
//     void TransitionToLightingPass(ID3D12GraphicsCommandList* pCommandList);
//     void TransitionToBasePass(ID3D12GraphicsCommandList* pCommandList);
//
//     ResourceView GetSrvTable() const { return mGBufferSrvTable; }
//     void DestroyRenderTargets();
// private:
//     bool CreateRenderTargets(uint32_t Width, uint32_t Height);
//
//     // Basic
//     ID3D12Device* mpDevice = nullptr;
//     D3D12MA::Allocator* mpAllocator = nullptr;
//
//     uint32_t mWidth = 0;
//     uint32_t mHeight = 0;
//
//     // GBuffer RenderTargets
//     FRenderTarget mSceneColor;
//     FRenderTarget mGBufferA;
//     FRenderTarget mGBufferB;
//     FRenderTarget mGBufferC;
//     FRenderTarget mDepthStencil;
//
//     // Global Descriptor Heaps
//     StaticResourceViewHeap* mSrvHeap = nullptr;
//     StaticResourceViewHeap* mRtvHeap = nullptr;
//     StaticResourceViewHeap* mDsvHeap = nullptr;
//
//     // Srv Table for GBuffers
//     ResourceView mGBufferSrvTable;
// };

#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12MemAlloc.h"
// 🟢 1. 引入我们的新架构类，删掉旧的 ResourceHeaps 和 ResourceView
#include "Renderer/D3D12Core/Descriptors/FDescriptorAllocation.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

struct FRenderTarget
{
    Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
    D3D12MA::Allocation* Allocation = nullptr;

    // 🟢 2. 使用新凭证代替旧的 ResourceView
    FDescriptorAllocation RtvView;
    DXGI_FORMAT Format;

    void Release()
    {
        pResource.Reset();
        if (Allocation)
        {
            Allocation->Release();
            Allocation = nullptr;
        }
        // 凭证自动释放坑位！
        RtvView.Free();
    }
};

class SceneRenderTargets
{
public:
    SceneRenderTargets() = default;
    ~SceneRenderTargets();

    // 🟢 3. 签名极大简化，不再需要传入一堆 Heap
    bool Initialize(FDevice* pDevice, D3D12MA::Allocator* pAllocator, uint32_t Width, uint32_t Height);

    void Resize(uint32_t Width, uint32_t Height);

    void BindBasePass(ID3D12GraphicsCommandList* pCommandList);
    void Clear(ID3D12GraphicsCommandList* pCommandList);

    void TransitionToLightingPass(ID3D12GraphicsCommandList* pCommandList);
    void TransitionToBasePass(ID3D12GraphicsCommandList* pCommandList);

    // 🟢 4. 获取纯 CPU 句柄，供光照阶段送入动态堆！
    D3D12_CPU_DESCRIPTOR_HANDLE GetSrvTableCpuHandle() const { return mGBufferSrvTable.GetCpuHandle(); }

    void DestroyRenderTargets();

private:
    bool CreateRenderTargets(uint32_t Width, uint32_t Height);

    FDevice* mpDevice = nullptr;
    D3D12MA::Allocator* mpAllocator = nullptr;

    uint32_t mWidth = 0;
    uint32_t mHeight = 0;

    FRenderTarget mSceneColor;
    FRenderTarget mGBufferA;
    FRenderTarget mGBufferB;
    FRenderTarget mGBufferC;
    FRenderTarget mDepthStencil;

    // 🟢 5. 存放这 5 张 G-Buffer 的 SRV 连续坑位凭证
    FDescriptorAllocation mGBufferSrvTable;
};