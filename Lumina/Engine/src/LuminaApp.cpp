#include "Engine/LuminaApp.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_dx12.h"
#include "ImGUI/imgui_impl_win32.h"

bool LuminaApp::InitializeApp(GraphicsDevice* pDevice, HWND Hwnd, uint32_t Width, uint32_t Height)
{
    mpGraphicsDevice = pDevice;
    mHwnd = Hwnd;
    mWidth = Width;
    mHeight = Height;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplWin32_Init(Hwnd);
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    mpGraphicsDevice->GetCbvSrvUavHeap().AllocateDescriptor(1, &mImGuiFontView);
    ImGui_ImplDX12_Init(
        mpGraphicsDevice->GetDevice().GetDevicePtr(), NUM_SWAPCHAIN_BACKBUFFER, DXGI_FORMAT_R8G8B8A8_UNORM,
        mpGraphicsDevice->GetCbvSrvUavHeap().GetHeap(),
        mImGuiFontView.GetCpuDescriptorHandle(),
        mImGuiFontView.GetGpuDescriptorHandle()
    );
    unsigned char* Pixels;
    int tWidth, tHeight;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&Pixels, &tWidth, &tHeight);

    return OnInit();
}

void LuminaApp::UpdateApp(double DeltaTime)
{
    // Camera Update
    mCamera.Update(DeltaTime);

    // ImGUI update
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    OnUpdate(DeltaTime);
}

void LuminaApp::RenderApp()
{
    // Begin Frame
    ID3D12GraphicsCommandList* pCommandList = mpGraphicsDevice->BeginFrame();

    OnRender(pCommandList);
    OnRenderUI();
    ImGui::Render();

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);

    // End Frame
    mpGraphicsDevice->EndFrameAndPresent();
}

void LuminaApp::DestroyApp()
{
    OnDestroy();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
