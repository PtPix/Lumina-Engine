#include "Editor/LuminaEditor.h"
#include "Editor/TestLayer/PBRModelTestLayer.h"
#include "ImGUI/imgui.h"
#include "Renderer/Renderer.h"
#include "Renderer/UI/UIRenderer.h"

bool LuminaEditor::OnInit()
{
    UIRenderer::Initialize(mHwnd, Renderer::GetD3D12Backend()->GetDevice());

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

void LuminaEditor::OnRender(FRenderGraph& RenderGraph)
{
    FViewportRenderTarget ViewportTarget;
    FRGTextureDesc ColorDesc = {};
    ColorDesc.Width = 1280;
    ColorDesc.Height = 720;
    ColorDesc.Format = Renderer::GetD3D12Backend()->GetBackBufferFormat();
    ColorDesc.Usage = ERGTextureUsage::RenderTarget | ERGTextureUsage::ShaderResource;
    ColorDesc.bUseClearValue = true;
    ColorDesc.ClearValue.Format = ColorDesc.Format;
    ColorDesc.ClearValue.Color[0] = 0.05f;
    ColorDesc.ClearValue.Color[1] = 0.05f;
    ColorDesc.ClearValue.Color[2] = 0.05f;
    ColorDesc.ClearValue.Color[3] = 1.0f;

    ViewportTarget.Color = RenderGraph.CreateTexture("EditorViewport.SceneColor", ColorDesc);
    FRGTextureDesc DepthDesc = {};
    DepthDesc.Width = 1280;
    DepthDesc.Height = 720;
    DepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.Usage = ERGTextureUsage::DepthStencil;
    DepthDesc.bUseClearValue = true;
    DepthDesc.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    DepthDesc.ClearValue.DepthStencil.Depth = 1.0f;

    ViewportTarget.Depth = RenderGraph.CreateTexture("EditorViewport.SceneDepth", DepthDesc);
    if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
    {
        mTestLayers[mActiveLayerIndex]->OnRender(RenderGraph);
    }
}

void LuminaEditor::OnRenderUI(FRenderGraph& RenderGraph)
{
    RenderEditorUI(RenderGraph);

    if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
    {
        mTestLayers[mActiveLayerIndex]->OnRenderUI();
    }
    FRGTextureHandle BackBuffer = Renderer::GetBackBufferHandle();
    FRGTextureHandle SceneColor = RenderGraph.GetTexture("EditorViewport.SceneColor");
    // RenderGraph.AddPass("EditorUI")
    //     .WriteRenderTarget(BackBuffer, ERGLoadOp::Load)
    //     .Execute([BackBuffer](FRenderGraphContext& Context)
    //     {
    //         UIRenderer::Render(
    //             Context.GetCommandContext(),
    //             Context.GetRTV(BackBuffer)
    //         );
    //     });
    auto UIPass = RenderGraph.AddPass("EditorUI");

    if (SceneColor.IsValid())
    {
        UIPass.ReadTexture(SceneColor);
    }

    UIPass
        .WriteRenderTarget(BackBuffer, ERGLoadOp::Load)
        .Execute([BackBuffer, SceneColor](FRenderGraphContext& Context)
        {
            if (SceneColor.IsValid())
            {
                UIRenderer::CopySRVToSlot(
                    Renderer::GetD3D12Backend()->GetDevice(),
                    Context.GetSRV(SceneColor),
                    UIRenderer::ViewportTextureSlot
                );
            }

            UIRenderer::Render(
                Context.GetCommandContext(),
                Context.GetRTV(BackBuffer)
            );
        });
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

void LuminaEditor::RenderEditorUI(FRenderGraph& RenderGraph)
{
        ImGuiViewport* Viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(Viewport->WorkPos);
    ImGui::SetNextWindowSize(Viewport->WorkSize);
    ImGui::SetNextWindowViewport(Viewport->ID);

    ImGuiWindowFlags RootFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_MenuBar;

    ImGui::Begin("LuminaEditorRoot", nullptr, RootFlags);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                PostQuitMessage(0);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    const float RightPanelWidth = 320.0f;
    const float Spacing = ImGui::GetStyle().ItemSpacing.x;

    ImVec2 ContentSize = ImGui::GetContentRegionAvail();
    ImVec2 ViewportSize = ImVec2(ContentSize.x - RightPanelWidth - Spacing, ContentSize.y);

    ImGui::BeginChild("ViewportRegion", ViewportSize, true, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::TextUnformatted("Viewport");

        ImVec2 Available = ImGui::GetContentRegionAvail();

        // 第一阶段可以先只保留区域。
        // 真正的场景贴图显示，需要后续把 Scene 渲染到 RenderTarget，再 ImGui::Image。
        ImGui::Image(
        UIRenderer::GetTextureID(UIRenderer::ViewportTextureSlot),
            Available,
            ImVec2(0, 0),
            ImVec2(1, 1)
        );
        ImGui::InvisibleButton("ViewportCanvas", Available);

        mViewportHovered = ImGui::IsItemHovered();
        mViewportSize = Available;
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("InspectorRegion", ImVec2(RightPanelWidth, ContentSize.y), true);
    {
        ImGui::TextUnformatted("Inspector");
        ImGui::Separator();

        if (mActiveLayerIndex >= 0 && mActiveLayerIndex < mTestLayers.size())
        {
            ImGui::Text("Layer: %s", mTestLayers[mActiveLayerIndex]->GetName().c_str());
            ImGui::Separator();

            mTestLayers[mActiveLayerIndex]->OnRenderUI();
        }

        ImGui::Separator();
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }
    ImGui::EndChild();

    ImGui::End();
}
