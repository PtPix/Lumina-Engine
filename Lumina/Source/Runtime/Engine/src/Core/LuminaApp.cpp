#include "Core/LuminaApp.h"

bool LuminaApp::InitializeApp(HWND Hwnd, uint32_t Width, uint32_t Height)
{
    mHwnd = Hwnd;
    mWidth = Width;
    mHeight = Height;

    return OnInit();
}

void LuminaApp::UpdateApp(double DeltaTime)
{
    OnUpdate(DeltaTime);
}

void LuminaApp::FixedUpdateApp(double FixedDeltaTime)
{
    OnFixedUpdate(FixedDeltaTime);
}

void LuminaApp::RenderApp(FRenderGraph& RenderGraph)
{
    OnRender(RenderGraph);
    OnRenderUI(RenderGraph);
}

void LuminaApp::DestroyApp()
{
    OnDestroy();
}
