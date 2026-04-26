#include "Engine/LuminaApp.h"

bool LuminaApp::InitializeApp(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    mHwnd = Hwnd;
    mWidth = Width;
    mHeight = Height;

    return OnInit();
}

void LuminaApp::UpdateApp(double DeltaTime)
{
    // // Camera Update
    // mCamera.Update(DeltaTime);

    OnUpdate(DeltaTime);
}

void LuminaApp::FixedUpdateApp(double FixedDeltaTime)
{
    OnFixedUpdate(FixedDeltaTime);
}

void LuminaApp::RenderApp(FCommandContext* pCommandContext)
{
    OnRender(pCommandContext);
    OnRenderUI(pCommandContext);
}

void LuminaApp::DestroyApp()
{
    OnDestroy();
}
