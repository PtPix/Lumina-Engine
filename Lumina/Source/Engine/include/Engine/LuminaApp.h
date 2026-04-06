#pragma once
#include "Camera.h"
#include "../../../Renderer/include/Renderer/D3D12Core/ResourceView.h"
#include "Renderer/D3D12Core/GraphicsDevice.h"

class LuminaRenderer;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;

class LuminaApp
{
public:
    virtual ~LuminaApp() = default;

    bool InitializeApp(GraphicsDevice* pDevice, HWND Hwnd, uint32_t Width, uint32_t Height);
    void UpdateApp(double DeltaTime);
    void RenderApp();
    void DestroyApp();

protected:

    virtual bool OnInit() = 0;
    virtual void OnUpdate(double DeltaTime) = 0;
    virtual void OnRender(ID3D12GraphicsCommandList* pCommandList) = 0;
    virtual void OnRenderUI() {}
    virtual void OnDestroy() = 0;

    GraphicsDevice* mpGraphicsDevice = nullptr;
    HWND mHwnd = nullptr;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;

    Camera mCamera;
    ResourceView mImGuiFontView;
};