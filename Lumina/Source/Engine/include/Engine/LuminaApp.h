#pragma once

#include <windows.h>
#include <cstdint>

class FCommandContext;

class LuminaApp
{
public:
    virtual ~LuminaApp() = default;

    bool InitializeApp(HWND Hwnd, uint32_t Width, uint32_t Height);
    void UpdateApp(double DeltaTime);
    void FixedUpdateApp(double FixedDeltaTime);
    void RenderApp(FCommandContext* pCommandContext);
    void DestroyApp();

protected:

    virtual bool OnInit() = 0;
    virtual void OnUpdate(double DeltaTime) = 0;
    virtual void OnFixedUpdate(double FixedDeltaTime) {}
    virtual void OnRender(FCommandContext* pCommandContext) = 0;
    virtual void OnRenderUI(FCommandContext* pCommandContext) {}
    virtual void OnDestroy() = 0;

    HWND mHwnd = nullptr;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
};