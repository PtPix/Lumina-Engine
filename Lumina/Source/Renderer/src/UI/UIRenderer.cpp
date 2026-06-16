#include <cassert>
#include <wrl/client.h>
#include <d3d12.h>

#include "Renderer/UI/UIRenderer.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_win32.h"
#include "ImGUI/imgui_impl_dx12.h"
#include "Renderer/D3D12Core/Common.h"

#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/D3D12Core/Core/Device.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND Hwnd,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
);
bool UIRenderer::ProcessWin32Message(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam, LRESULT& OutResult)
{
    if (!mbInitialized || ImGui::GetCurrentContext() == nullptr)
    {
        return false;
    }

    if (ImGui_ImplWin32_WndProcHandler(Hwnd, Message, WParam, LParam))
    {
        OutResult = 1;
        return true;
    }

    return false;
}

bool UIRenderer::mbInitialized = false;

static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mImGuiSrvHeap = nullptr;
static UINT mImGuiSrvDescriptorSize = 0;
ImTextureID UIRenderer::GetTextureID(uint32_t Slot)
{
    D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle =
        mImGuiSrvHeap->GetGPUDescriptorHandleForHeapStart();

    GpuHandle.ptr += static_cast<SIZE_T>(Slot) * mImGuiSrvDescriptorSize;

    return static_cast<ImTextureID>(GpuHandle.ptr);
}

void UIRenderer::CopySRVToSlot(FDevice* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE SrcSRV, uint32_t Slot)
{
    D3D12_CPU_DESCRIPTOR_HANDLE DstCPU =
        mImGuiSrvHeap->GetCPUDescriptorHandleForHeapStart();

    DstCPU.ptr += static_cast<SIZE_T>(Slot) * mImGuiSrvDescriptorSize;

    pDevice->GetDevice()->CopyDescriptorsSimple(
        1,
        DstCPU,
        SrcSRV,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
}

void UIRenderer::Initialize(HWND Hwnd, FDevice* pDevice)
{
    if (mbInitialized) return;

    // ID3D12Device* pDevice = pDevice->GetDevice();
    assert(pDevice != nullptr);

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
    HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    HeapDesc.NumDescriptors = 64;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HeapDesc.NodeMask = 0;

    HRESULT HResult = pDevice->GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&mImGuiSrvHeap));
    assert(SUCCEEDED(HResult));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplWin32_Init(Hwnd);

    ImGui_ImplDX12_Init(
        pDevice->GetDevice(),
        NUM_SWAPCHAIN_BACKBUFFER,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        mImGuiSrvHeap.Get(),
        mImGuiSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        mImGuiSrvHeap->GetGPUDescriptorHandleForHeapStart()
        );

    unsigned char* Pixels;
    int tWidth, tHeight;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&Pixels, &tWidth, &tHeight);

    mImGuiSrvDescriptorSize =
    pDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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

void UIRenderer::Render(FCommandContext* pCommandContext, D3D12_CPU_DESCRIPTOR_HANDLE Rtv)
{
    if (!mbInitialized) return;

    ImGui::Render();

    ID3D12DescriptorHeap* ppHeaps[] = { mImGuiSrvHeap.Get() };
    pCommandContext->SetDescriptorHeaps(1, ppHeaps);
    pCommandContext->SetRenderTargets(1, &Rtv, nullptr);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandContext->GetCommandList());
}