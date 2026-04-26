#include <cassert>
#include <wrl/client.h>
#include <d3d12.h>

#include "Renderer/UI/UIRenderer.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_win32.h"
#include "ImGUI/imgui_impl_dx12.h"
#include "Renderer/D3D12Core/Common.h"

#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"
#include "Renderer/D3D12Core/Core/FDevice.h"

bool UIRenderer::mbInitialized = false;

static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mImGuiSrvHeap = nullptr;

void UIRenderer::Initialize(HWND Hwnd)
{
    if (mbInitialized) return;

    ID3D12Device* pDevice = D3D12Backend::GetDevice()->GetDevice();
    assert(pDevice != nullptr);

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
    HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    HeapDesc.NumDescriptors = 64;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HeapDesc.NodeMask = 0;

    HRESULT HResult = pDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&mImGuiSrvHeap));
    assert(SUCCEEDED(HResult));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplWin32_Init(Hwnd);

    ImGui_ImplDX12_Init(
        pDevice,
        NUM_SWAPCHAIN_BACKBUFFER,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        mImGuiSrvHeap.Get(),
        mImGuiSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        mImGuiSrvHeap->GetGPUDescriptorHandleForHeapStart()
        );

    unsigned char* Pixels;
    int tWidth, tHeight;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&Pixels, &tWidth, &tHeight);

    mbInitialized = true;
}

void UIRenderer::Shutdown()
{
    if (!mbInitialized) return;

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    mImGuiSrvHeap.Reset();
    mbInitialized = false;
}

void UIRenderer::BeginFrame()
{
    if (!mbInitialized) return;

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void UIRenderer::Render(FCommandContext* pCommandContext)
{
    if (!mbInitialized) return;

    ImGui::Render();

    ID3D12DescriptorHeap* ppHeaps[] = { mImGuiSrvHeap.Get() };
    pCommandContext->SetDescriptorHeaps(1, ppHeaps);
    D3D12_CPU_DESCRIPTOR_HANDLE RTV = D3D12Backend::GetCurrentBackBufferRTV();
    pCommandContext->SetRenderTargets(1, &RTV, nullptr);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandContext->GetCommandList());
}