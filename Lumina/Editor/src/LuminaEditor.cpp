#include "Editor/LuminaEditor.h"
#include "Editor/TestLayer/PBRModelTestLayer.h"
#include "ImGUI/imgui.h"
#include "Renderer/UI/UIRenderer.h"

bool LuminaEditor::OnInit()
{
    UIRenderer::Initialize(mHwnd);

    RegisterTestLayer<PBRModelTestLayer>();

    if (!mTestLayers.empty())
    {
        mActiveLayerIndex = 0;
        mTestLayers[mActiveLayerIndex]->OnAttach();
    }

    return true;
}

void LuminaEditor::OnUpdate(double DeltaTime)
{
    UIRenderer::BeginFrame();
    if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
    {
        mTestLayers[mActiveLayerIndex]->OnUpdate(DeltaTime);
    }
}

void LuminaEditor::OnFixedUpdate(double FixedDeltaTime)
{
    if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
    {
        mTestLayers[mActiveLayerIndex]->OnFixedUpdate(FixedDeltaTime);
    }
}

void LuminaEditor::OnRender(FCommandContext* pCommandContext)
{
    if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
    {
        mTestLayers[mActiveLayerIndex]->OnRender(pCommandContext);
    }

}

void LuminaEditor::OnRenderUI(FCommandContext* pCommandContext)
{
    RenderEditorUI();

    if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
    {
        mTestLayers[mActiveLayerIndex]->OnRenderUI();
    }
    UIRenderer::Render(pCommandContext);
}

void LuminaEditor::OnDestroy()
{
    UIRenderer::Shutdown();
    if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
    {
        mTestLayers[mActiveLayerIndex]->OnDetach();
    }
    mTestLayers.clear();
}

void LuminaEditor::RenderEditorUI()
{
    ImGui::DockSpaceOverViewport();

    // 绘制主菜单
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { PostQuitMessage(0); } // 退出引擎
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // 绘制测试用例选择面板
    ImGui::Begin("Test Modules");
    for (int i = 0; i < mTestLayers.size(); ++i)
    {
        bool isSelected = (mActiveLayerIndex == i);
        if (ImGui::Selectable(mTestLayers[i]->GetName().c_str(), isSelected))
        {
            // 切换测试层
            if (mActiveLayerIndex != i)
            {
                if (mActiveLayerIndex >= 0) mTestLayers[mActiveLayerIndex]->OnDetach();
                mActiveLayerIndex = i;
                mTestLayers[mActiveLayerIndex]->OnAttach();
            }
        }
    }
    ImGui::End();

    // 绘制全局性能监控
    ImGui::Begin("Profiler");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Delta Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    // 这里未来可以对接你的 mTimer 或 GPU Profiler
    ImGui::End();
}
