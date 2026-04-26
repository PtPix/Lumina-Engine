#include "Editor/TestLayer/PBRModelTestLayer.h"
#include <d3d12.h>
#include "ImGUI/imgui.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/GraphicsDevice.h"
#include "Renderer/D3D12Core/Core/FCommandContext.h"

void PBRModelTestLayer::OnAttach()
{

}

void PBRModelTestLayer::OnDetach()
{

}

void PBRModelTestLayer::OnUpdate(double DeltaTime)
{

}

void PBRModelTestLayer::OnRender(FCommandContext* pCommandContext)
{
    ID3D12GraphicsCommandList* pCmdList = pCommandContext->GetCommandList();

    // 1. 资源屏障转换 (Resource Barrier)
    // 确保你的 BackBuffer 从 PRESENT 状态转换到了 RENDER_TARGET 状态
    // 如果你在 mGraphicsDevice.BeginFrame() 里已经做了这件事，这一步可以省略。
    /* pCommandContext->TransitionResource(
        CurrentBackBuffer,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    */

    // 2. 获取当前帧的 Render Target View (RTV)
    // 假设你能从你的 GraphicsDevice 或 RenderBackend 拿到它
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV = D3D12Backend::GetCurrentBackBufferRTV();

    // 3. 绑定 Render Target
    // 告诉 DX12：接下来的所有操作，都画在这个 Render Target 上
    pCmdList->OMSetRenderTargets(1, &backBufferRTV, FALSE, nullptr);

    // 4. 清理背景颜色 (非常重要！防止 UI 重影)
    // 这里设定一个深灰色背景
    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    pCmdList->ClearRenderTargetView(backBufferRTV, clearColor, 0, nullptr);
    // pCmdList->ClearDepthStencilView(depthStencilDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // 5. 设置视口 (Viewport) 和 裁剪矩形 (Scissor Rect)
    // 就算只画 UI，DX12 也需要明确告诉它可绘制区域的大小
    float Width = 1280;
    float Height = 720;
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)Width, (LONG)Height };
    pCmdList->RSSetViewports(1, &viewport);
    pCmdList->RSSetScissorRects(1, &scissorRect);
}

void PBRModelTestLayer::OnRenderUI()
{
    // ImGui::Begin("Model Properties");
    //
    // ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    //
    // ImGui::End();
}
