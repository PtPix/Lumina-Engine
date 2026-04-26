// #include "Renderer/Rendering/SceneRenderTargets.h"
//
// SceneRenderTargets::~SceneRenderTargets()
// {
// }
//
// bool SceneRenderTargets::Initialize(ID3D12Device* pDevice, D3D12MA::Allocator* pAllocator,
//         StaticResourceViewHeap* pRtvHeap, StaticResourceViewHeap* pSrvHeap,
//         StaticResourceViewHeap* pDsvHeap,
//         uint32_t Width, uint32_t Height)
// {
//     mpDevice = pDevice;
//     mpAllocator = pAllocator;
//     mSrvHeap = pSrvHeap;
//     mRtvHeap = pRtvHeap;
//     mDsvHeap = pDsvHeap;
//
//     Resize(Width, Height);
//     return true;
// }
//
// void SceneRenderTargets::Resize(uint32_t Width, uint32_t Height)
// {
//     if (mWidth == Width && mHeight == Height)
//     {
//         return;
//     }
//
//     mWidth = Width;
//     mHeight = Height;
//
//     DestroyRenderTargets();
//     CreateRenderTargets(mWidth, mHeight);
// }
//
// void SceneRenderTargets::BindBasePass(ID3D12GraphicsCommandList* pCommandList)
// {
//     D3D12_CPU_DESCRIPTOR_HANDLE Rtvs[4] = {
//         mSceneColor.RtvView.GetCpuDescriptorHandle(),
//         mGBufferA.RtvView.GetCpuDescriptorHandle(),
//         mGBufferB.RtvView.GetCpuDescriptorHandle(),
//         mGBufferC.RtvView.GetCpuDescriptorHandle(),
//     };
//     D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = mDepthStencil.RtvView.GetCpuDescriptorHandle();
//     pCommandList->OMSetRenderTargets(4, Rtvs, FALSE, &DsvHandle);
// }
//
// void SceneRenderTargets::Clear(ID3D12GraphicsCommandList* pCommandList)
// {
//     const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
//
//     pCommandList->ClearRenderTargetView(mSceneColor.RtvView.GetCpuDescriptorHandle(), ClearColor, 0, nullptr);
//     pCommandList->ClearRenderTargetView(mGBufferA.RtvView.GetCpuDescriptorHandle(), ClearColor, 0, nullptr);
//     pCommandList->ClearRenderTargetView(mGBufferB.RtvView.GetCpuDescriptorHandle(), ClearColor, 0, nullptr);
//     pCommandList->ClearRenderTargetView(mGBufferC.RtvView.GetCpuDescriptorHandle(), ClearColor, 0, nullptr);
//
//     pCommandList->ClearDepthStencilView(mDepthStencil.RtvView.GetCpuDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
// }
//

//
// bool SceneRenderTargets::CreateRenderTargets(uint32_t Width, uint32_t Height)
// {
//     // Create RenderTargets Descriptors ans ShaderResource Views
//     D3D12_RESOURCE_DESC RTDesc = {};
//     RTDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//     RTDesc.Alignment = 0;
//     RTDesc.Width = Width;
//     RTDesc.Height = Height;
//     RTDesc.DepthOrArraySize = 1;
//     RTDesc.MipLevels = 1;
//     RTDesc.SampleDesc.Count = 1;
//     RTDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//     RTDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
//
//     D3D12MA::ALLOCATION_DESC AllocDesc = {};
//     AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
//
//     D3D12_CLEAR_VALUE ClearColor = {};
//     ClearColor.Color[0] = 0.0f; ClearColor.Color[1] = 0.0f;
//     ClearColor.Color[2] = 0.0f; ClearColor.Color[3] = 0.0f;
//
//     mSrvHeap->AllocateDescriptor(5, &mGBufferSrvTable);
//     D3D12_CPU_DESCRIPTOR_HANDLE CurrentSrvHandle = mGBufferSrvTable.GetCpuDescriptorHandle();
//     UINT SrvDescSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//     auto CreateGBuffer = [&](FRenderTarget& Target, DXGI_FORMAT Format, const wchar_t* Name)
//     {
//         Target.Format = Format;
//         RTDesc.Format = Format;
//         ClearColor.Format = Format;
//
//         mpAllocator->CreateResource(
//             &AllocDesc, &RTDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &ClearColor,
//             &Target.Allocation, IID_PPV_ARGS(&Target.pResource));
//         Target.pResource->SetName(Name);
//
//         mRtvHeap->AllocateDescriptor(1, &Target.RtvView);
//
//         mpDevice->CreateRenderTargetView(Target.pResource.Get(), nullptr, Target.RtvView.GetCpuDescriptorHandle());
//
//         D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
//         SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//         SRVDesc.Format = Format;
//         SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//         SRVDesc.Texture2D.MipLevels = 1;
//         mpDevice->CreateShaderResourceView(Target.pResource.Get(), &SRVDesc, CurrentSrvHandle);
//         CurrentSrvHandle.ptr += SrvDescSize;
//     };
//
//     CreateGBuffer(mSceneColor, DXGI_FORMAT_R16G16B16A16_FLOAT, L"GBuffer_SceneColor");
//     CreateGBuffer(mGBufferA,   DXGI_FORMAT_R16G16B16A16_FLOAT, L"GBuffer_A_Normal");
//     CreateGBuffer(mGBufferB,   DXGI_FORMAT_R8G8B8A8_UNORM,     L"GBuffer_B_Material");
//     CreateGBuffer(mGBufferC,   DXGI_FORMAT_R8G8B8A8_UNORM,     L"GBuffer_C_AlbedoAO");
//
//     // Create DSV and Srv
//     D3D12_RESOURCE_DESC DepthDesc = RTDesc;
//     DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//     DepthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
//
//     D3D12_CLEAR_VALUE DepthClear = {};
//     DepthClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//     DepthClear.DepthStencil.Depth = 1.0f;
//     DepthClear.DepthStencil.Stencil = 0;
//     mDepthStencil.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//
//     mpAllocator->CreateResource(
//         &AllocDesc, &DepthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &DepthClear,
//         &mDepthStencil.Allocation, IID_PPV_ARGS(&mDepthStencil.pResource));
//     mDepthStencil.pResource->SetName(L"GBuffer_DepthStencil");
//     mDsvHeap->AllocateDescriptor(1, &mDepthStencil.RtvView);
//
//     D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
//     DsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//     DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
//     DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
//     mpDevice->CreateDepthStencilView(mDepthStencil.pResource.Get(), &DsvDesc, mDepthStencil.RtvView.GetCpuDescriptorHandle());
//
//     D3D12_SHADER_RESOURCE_VIEW_DESC DepthSrvDesc = {};
//     DepthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//     DepthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
//     DepthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//     DepthSrvDesc.Texture2D.MipLevels = 1;
//     mpDevice->CreateShaderResourceView(mDepthStencil.pResource.Get(), &DepthSrvDesc, CurrentSrvHandle);
//
//     return true;
// }
//
// void SceneRenderTargets::DestroyRenderTargets()
// {
//     if (mSrvHeap && mGBufferSrvTable.GetCpuDescriptorHandle().ptr != 0)
//     {
//         mSrvHeap->FreeDescriptor(&mGBufferSrvTable);
//     }
//
//     if (mRtvHeap)
//     {
//         if (mSceneColor.RtvView.GetCpuDescriptorHandle().ptr != 0)
//         {
//             mRtvHeap->FreeDescriptor(&mSceneColor.RtvView);
//         }
//         if (mGBufferA.RtvView.GetCpuDescriptorHandle().ptr != 0)
//         {
//             mRtvHeap->FreeDescriptor(&mGBufferA.RtvView);
//         }
//         if (mGBufferB.RtvView.GetCpuDescriptorHandle().ptr != 0)
//         {
//             mRtvHeap->FreeDescriptor(&mGBufferB.RtvView);
//         }
//         if (mGBufferC.RtvView.GetCpuDescriptorHandle().ptr != 0)
//         {
//             mRtvHeap->FreeDescriptor(&mGBufferC.RtvView);
//         }
//     }
//
//     if (mDsvHeap && mDepthStencil.RtvView.GetCpuDescriptorHandle().ptr != 0)
//     {
//         mDsvHeap->FreeDescriptor(&mDepthStencil.RtvView);
//     }
//
//     mSceneColor.Release();
//     mGBufferA.Release();
//     mGBufferB.Release();
//     mGBufferC.Release();
//     mDepthStencil.Release();
// }

#include "Renderer/Rendering/SceneRenderTargets.h"

void SceneRenderTargets::TransitionToLightingPass(ID3D12GraphicsCommandList* pCommandList)
{
    D3D12_RESOURCE_BARRIER Barriers[5] = {};

    auto SetupBarrier = [](D3D12_RESOURCE_BARRIER& Barrier, ID3D12Resource* pResource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
    {
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        Barrier.Transition.pResource = pResource;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = StateBefore;
        Barrier.Transition.StateAfter = StateAfter;
    };

    SetupBarrier(Barriers[0], mSceneColor.pResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    SetupBarrier(Barriers[1], mGBufferA.pResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    SetupBarrier(Barriers[2], mGBufferB.pResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    SetupBarrier(Barriers[3], mGBufferC.pResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    SetupBarrier(Barriers[4], mDepthStencil.pResource.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    pCommandList->ResourceBarrier(5, Barriers);
}

void SceneRenderTargets::TransitionToBasePass(ID3D12GraphicsCommandList* pCommandList)
{
    D3D12_RESOURCE_BARRIER Barriers[5] = {};

    auto SetupBarrier = [](D3D12_RESOURCE_BARRIER& Barrier, ID3D12Resource* pResource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
    {
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        Barrier.Transition.pResource = pResource;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = StateBefore;
        Barrier.Transition.StateAfter = StateAfter;
    };

    SetupBarrier(Barriers[0], mSceneColor.pResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetupBarrier(Barriers[1], mGBufferA.pResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetupBarrier(Barriers[2], mGBufferB.pResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetupBarrier(Barriers[3], mGBufferC.pResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetupBarrier(Barriers[4], mDepthStencil.pResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    pCommandList->ResourceBarrier(5, Barriers);
}

SceneRenderTargets::~SceneRenderTargets()
{
    DestroyRenderTargets();
}

bool SceneRenderTargets::Initialize(FDevice* pDevice, D3D12MA::Allocator* pAllocator, uint32_t Width, uint32_t Height)
{
    mpDevice = pDevice;
    mpAllocator = pAllocator;

    Resize(Width, Height);
    return true;
}

void SceneRenderTargets::Resize(uint32_t Width, uint32_t Height)
{
    if (mWidth == Width && mHeight == Height) return;

    mWidth = Width;
    mHeight = Height;

    DestroyRenderTargets();
    CreateRenderTargets(mWidth, mHeight);
}

void SceneRenderTargets::BindBasePass(ID3D12GraphicsCommandList* pCommandList)
{
    // 🟢 直接调用 GetCpuHandle()
    D3D12_CPU_DESCRIPTOR_HANDLE Rtvs[4] = {
        mSceneColor.RtvView.GetCpuHandle(),
        mGBufferA.RtvView.GetCpuHandle(),
        mGBufferB.RtvView.GetCpuHandle(),
        mGBufferC.RtvView.GetCpuHandle(),
    };
    D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = mDepthStencil.RtvView.GetCpuHandle();
    pCommandList->OMSetRenderTargets(4, Rtvs, FALSE, &DsvHandle);
}

void SceneRenderTargets::Clear(ID3D12GraphicsCommandList* pCommandList)
{
    const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    pCommandList->ClearRenderTargetView(mSceneColor.RtvView.GetCpuHandle(), ClearColor, 0, nullptr);
    pCommandList->ClearRenderTargetView(mGBufferA.RtvView.GetCpuHandle(), ClearColor, 0, nullptr);
    pCommandList->ClearRenderTargetView(mGBufferB.RtvView.GetCpuHandle(), ClearColor, 0, nullptr);
    pCommandList->ClearRenderTargetView(mGBufferC.RtvView.GetCpuHandle(), ClearColor, 0, nullptr);

    pCommandList->ClearDepthStencilView(mDepthStencil.RtvView.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

// // ... TransitionToLightingPass 和 TransitionToBasePass 的代码完全不变，省略 ...
// void SceneRenderTargets::TransitionToLightingPass(ID3D12GraphicsCommandList* pCommandList)
// {
//
// }
// void SceneRenderTargets::TransitionToBasePass(ID3D12GraphicsCommandList* pCommandList)
// {
//
// }

bool SceneRenderTargets::CreateRenderTargets(uint32_t Width, uint32_t Height)
{
    D3D12_RESOURCE_DESC RTDesc = {};
    RTDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    RTDesc.Alignment = 0;
    RTDesc.Width = Width;
    RTDesc.Height = Height;
    RTDesc.DepthOrArraySize = 1;
    RTDesc.MipLevels = 1;
    RTDesc.SampleDesc.Count = 1;
    RTDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    RTDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12MA::ALLOCATION_DESC AllocDesc = {};
    AllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE ClearColor = {};
    ClearColor.Color[0] = 0.0f; ClearColor.Color[1] = 0.0f;
    ClearColor.Color[2] = 0.0f; ClearColor.Color[3] = 0.0f;

    // 🟢 核心魔改 1：向 Device 申请 5 个连续的 SRV 坑位，并拿到起点指针
    mGBufferSrvTable = mpDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->Allocate(5);
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentSrvHandle = mGBufferSrvTable.GetCpuHandle();
    UINT SrvDescSize = mpDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto CreateGBuffer = [&](FRenderTarget& Target, DXGI_FORMAT Format, const wchar_t* Name)
    {
        Target.Format = Format;
        RTDesc.Format = Format;
        ClearColor.Format = Format;

        mpAllocator->CreateResource(
            &AllocDesc, &RTDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &ClearColor,
            &Target.Allocation, IID_PPV_ARGS(&Target.pResource));
        Target.pResource->SetName(Name);

        // 🟢 核心魔改 2：向 Device 申请 1 个 RTV 坑位
        Target.RtvView = mpDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->Allocate(1);
        mpDevice->GetDevice()->CreateRenderTargetView(Target.pResource.Get(), nullptr, Target.RtvView.GetCpuHandle());

        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SRVDesc.Format = Format;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;

        // 使用连续指针创建 SRV，并把指针向后推
        mpDevice->GetDevice()->CreateShaderResourceView(Target.pResource.Get(), &SRVDesc, CurrentSrvHandle);
        CurrentSrvHandle.ptr += SrvDescSize;
    };

    CreateGBuffer(mSceneColor, DXGI_FORMAT_R16G16B16A16_FLOAT, L"GBuffer_SceneColor");
    CreateGBuffer(mGBufferA,   DXGI_FORMAT_R16G16B16A16_FLOAT, L"GBuffer_A_Normal");
    CreateGBuffer(mGBufferB,   DXGI_FORMAT_R8G8B8A8_UNORM,     L"GBuffer_B_Material");
    CreateGBuffer(mGBufferC,   DXGI_FORMAT_R8G8B8A8_UNORM,     L"GBuffer_C_AlbedoAO");

    // 深度缓冲
    D3D12_RESOURCE_DESC DepthDesc = RTDesc;
    DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE DepthClear = {};
    DepthClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthClear.DepthStencil.Depth = 1.0f;
    DepthClear.DepthStencil.Stencil = 0;
    mDepthStencil.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    mpAllocator->CreateResource(
        &AllocDesc, &DepthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &DepthClear,
        &mDepthStencil.Allocation, IID_PPV_ARGS(&mDepthStencil.pResource));
    mDepthStencil.pResource->SetName(L"GBuffer_DepthStencil");

    // 🟢 核心魔改 3：向 Device 申请 1 个 DSV 坑位
    mDepthStencil.RtvView = mpDevice->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->Allocate(1);

    D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
    DsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    mpDevice->GetDevice()->CreateDepthStencilView(mDepthStencil.pResource.Get(), &DsvDesc, mDepthStencil.RtvView.GetCpuHandle());

    // 深度缓冲的 SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC DepthSrvDesc = {};
    DepthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    DepthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    DepthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    DepthSrvDesc.Texture2D.MipLevels = 1;
    mpDevice->GetDevice()->CreateShaderResourceView(mDepthStencil.pResource.Get(), &DepthSrvDesc, CurrentSrvHandle);

    return true;
}

void SceneRenderTargets::DestroyRenderTargets()
{
    // 🟢 借助 RAII 和封装好的 Free 函数，释放变得极其干净
    mGBufferSrvTable.Free();

    mSceneColor.Release();
    mGBufferA.Release();
    mGBufferB.Release();
    mGBufferC.Release();
    mDepthStencil.Release();
}