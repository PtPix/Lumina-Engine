#include "LuminaEditor.h"
#include "ImGUI/imgui.h"
#include "PbrModelDemo/PbrModelDemo.h"
#include "Renderer.h"
#include "RenderCore.h"
#include "UIRenderer.h"
#include "D3D12Backend.h"

bool LuminaEditor::OnInit()
{
    UIRenderer::Initialize(mHwnd, FRendererCore::GetDevice());

    RegisterTestLayer<PbrModelDemo>();

    if (!mRenderDemos.empty())
    {
        mActiveDemoIndex = 0;
        mRenderDemos[mActiveDemoIndex]->OnAttach();
    }

    return true;
}

void LuminaEditor::OnUpdate(double DeltaTime)
{
    UIRenderer::BeginFrame();

    if (mActiveDemoIndex >= 0 && mActiveDemoIndex < mRenderDemos.size())
    {
        mRenderDemos[mActiveDemoIndex]->OnUpdate(DeltaTime);
    }
}

void LuminaEditor::OnFixedUpdate(double FixedDeltaTime)
{
}

void LuminaEditor::OnRender(FRenderGraph& RenderGraph)
{
    FRGTextureDesc ColorDesc = {};
    ColorDesc.Width = 1280;
    ColorDesc.Height = 720;
    ColorDesc.Format = FRendererCore::GetBackend()->GetBackBufferFormat();
    ColorDesc.Usage = ERGTextureUsage::RenderTarget | ERGTextureUsage::ShaderResource;
    ColorDesc.bUseClearValue = true;
    ColorDesc.ClearValue.Format = ColorDesc.Format;
    ColorDesc.ClearValue.Color[0] = 0.05f;
    ColorDesc.ClearValue.Color[1] = 0.05f;
    ColorDesc.ClearValue.Color[2] = 0.05f;
    ColorDesc.ClearValue.Color[3] = 1.0f;

    RenderGraph.CreateTexture("EditorViewport.SceneColor", ColorDesc);

    if (mActiveDemoIndex >= 0 && mActiveDemoIndex < mRenderDemos.size())
    {
        mRenderDemos[mActiveDemoIndex]->OnRender(RenderGraph);
    }
}

void LuminaEditor::OnRenderUI(FRenderGraph& RenderGraph)
{
    RenderEditorUI(RenderGraph);

    FRGTextureHandle BackBuffer = Renderer::GetBackBufferHandle();
    // FRGTextureHandle SceneColor = RenderGraph.GetTexture("EditorViewport.SceneColor");
    FRGTextureHandle SceneColor = RenderGraph.GetTexture("Debug.VisualizedDepth");

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
                    FRendererCore::GetDevice(),
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
    if (mActiveDemoIndex >= 0 && mActiveDemoIndex < mRenderDemos.size())
    {
        mRenderDemos[mActiveDemoIndex]->OnDetach();
    }
    mRenderDemos.clear();
}

void LuminaEditor::SetActiveLayer(int16_t ActiveLayer)
{
    assert(ActiveLayer >= 0 && ActiveLayer < mRenderDemos.size());
    if (mActiveDemoIndex >= 0 && mActiveDemoIndex < mRenderDemos.size())
    {
        mRenderDemos[mActiveDemoIndex]->OnDetach();
    }
    mRenderDemos[ActiveLayer]->OnAttach();
    mActiveDemoIndex = ActiveLayer;
}

void LuminaEditor::RenderEditorUI(FRenderGraph& RenderGraph)
{
    ImGuiViewport* Viewport = ImGui::GetMainViewport();
    if (!Viewport)
    {
        return;
    }

    const ImVec4 Background = ImVec4(0.045f, 0.052f, 0.062f, 1.0f);
    const ImVec4 Panel = ImVec4(0.075f, 0.086f, 0.102f, 1.0f);
    const ImVec4 PanelSoft = ImVec4(0.095f, 0.108f, 0.128f, 1.0f);
    const ImVec4 PanelHover = ImVec4(0.130f, 0.150f, 0.178f, 1.0f);
    const ImVec4 Accent = ImVec4(0.290f, 0.590f, 0.980f, 1.0f);
    const ImVec4 AccentSoft = ImVec4(0.180f, 0.360f, 0.620f, 1.0f);
    const ImVec4 Text = ImVec4(0.880f, 0.910f, 0.950f, 1.0f);
    const ImVec4 TextMuted = ImVec4(0.520f, 0.570f, 0.640f, 1.0f);
    const ImVec4 Border = ImVec4(0.160f, 0.180f, 0.215f, 1.0f);
    const ImVec4 Success = ImVec4(0.360f, 0.820f, 0.540f, 1.0f);

    int StyleVarCount = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); ++StyleVarCount;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9.0f, 5.0f)); ++StyleVarCount;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 7.0f)); ++StyleVarCount;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); ++StyleVarCount;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f); ++StyleVarCount;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f); ++StyleVarCount;
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 4.0f); ++StyleVarCount;

    int ColorCount = 0;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Background); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Panel); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_PopupBg, Panel); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_Border, Border); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, PanelSoft); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, PanelHover); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, AccentSoft); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.060f, 0.070f, 0.083f, 1.0f)); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.105f, 0.125f, 0.150f, 1.0f)); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.155f, 0.190f, 0.230f, 1.0f)); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, AccentSoft); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.105f, 0.125f, 0.150f, 1.0f)); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.150f, 0.185f, 0.230f, 1.0f)); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, AccentSoft); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_Text, Text); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, TextMuted); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_Separator, Border); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, Panel); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.190f, 0.220f, 0.260f, 1.0f)); ++ColorCount;
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.260f, 0.300f, 0.360f, 1.0f)); ++ColorCount;

    auto DrawSectionHeader = [&](const char* Label)
    {
        ImGui::Spacing();
        ImGui::TextColored(Accent, "%s", Label);
        ImGui::Separator();
    };

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
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_MenuBar;

    if (ImGui::Begin("LuminaEditorRoot", nullptr, RootFlags))
    {
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
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Scene", nullptr, true, false);
                ImGui::MenuItem("Viewport", nullptr, true, false);
                ImGui::MenuItem("Inspector", nullptr, true, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Render Graph", nullptr, false, false);
                ImGui::MenuItem("GPU Profiler", nullptr, false, false);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::BeginChild("TopToolbar", ImVec2(0.0f, 46.0f), false, ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::SetCursorPos(ImVec2(16.0f, 12.0f));
            ImGui::TextColored(Accent, "Lumina");
            ImGui::SameLine();
            ImGui::TextDisabled("DX12 RenderGraph");

            ImGui::SameLine(210.0f);
            ImGui::Button("Select", ImVec2(70.0f, 26.0f));
            ImGui::SameLine();
            ImGui::Button("Move", ImVec2(64.0f, 26.0f));
            ImGui::SameLine();
            ImGui::Button("Rotate", ImVec2(70.0f, 26.0f));
            ImGui::SameLine();
            ImGui::Button("Scale", ImVec2(64.0f, 26.0f));

            const float RightGroupWidth = 220.0f;
            float RightStart = ImGui::GetWindowWidth() - RightGroupWidth;
            if (RightStart > ImGui::GetCursorPosX())
            {
                ImGui::SameLine(RightStart);
            }
            ImGui::TextColored(Success, "Ready");
            ImGui::SameLine();
            ImGui::TextDisabled("| %.1f FPS", ImGui::GetIO().Framerate);
        }
        ImGui::EndChild();

        const float StatusBarHeight = 28.0f;
        const float Gutter = 8.0f;
        const float LeftPanelWidth = 250.0f;
        const float RightPanelWidth = 330.0f;

        ImVec2 ContentSize = ImGui::GetContentRegionAvail();
        ImVec2 MainSize = ImVec2(ContentSize.x, ContentSize.y - StatusBarHeight - Gutter);
        if (MainSize.y < 120.0f)
        {
            MainSize.y = 120.0f;
        }

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + Gutter);
        ImGui::BeginChild("MainWorkspace", ImVec2(ContentSize.x - Gutter * 2.0f, MainSize.y), false, ImGuiWindowFlags_NoScrollbar);
        {
            float CenterPanelWidth = ImGui::GetContentRegionAvail().x - LeftPanelWidth - RightPanelWidth - Gutter * 2.0f;
            if (CenterPanelWidth < 320.0f)
            {
                CenterPanelWidth = 320.0f;
            }

            ImGui::BeginChild("ScenePanel", ImVec2(LeftPanelWidth, 0.0f), true);
            {
                DrawSectionHeader("Scene");

                if (ImGui::TreeNodeEx("World", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
                {
                    ImGui::TextDisabled("Camera");
                    ImGui::TextDisabled("Directional Light");
                    ImGui::TextDisabled("Environment");
                    ImGui::TreePop();
                }

                DrawSectionHeader("Demos");
                for (int i = 0; i < (int)mRenderDemos.size(); ++i)
                {
                    ImGui::PushID(i);
                    if (ImGui::Selectable(mRenderDemos[i]->GetName().c_str(), i == mActiveDemoIndex))
                    {
                        SetActiveLayer((int16_t)i);   // 内部 OnDetach 旧 / OnAttach 新(你已实现)
                    }
                    ImGui::PopID();
                }

                DrawSectionHeader("Assets");
                ImGui::TextDisabled("Materials");
                ImGui::TextDisabled("Meshes");
                ImGui::TextDisabled("Textures");
            }
            ImGui::EndChild();

            ImGui::SameLine(0.0f, Gutter);

            ImGui::BeginChild("ViewportPanel", ImVec2(CenterPanelWidth, 0.0f), true, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::TextColored(Accent, "Viewport");
                ImGui::SameLine();
                ImGui::TextDisabled("%.0f x %.0f", mViewportSize.x, mViewportSize.y);
                ImGui::SameLine();
                ImGui::TextDisabled("| SceneColor: %s",
                    RenderGraph.GetTexture("EditorViewport.SceneColor").IsValid() ? "Valid" : "Missing");
                ImGui::Separator();

                ImVec2 Available = ImGui::GetContentRegionAvail();
                if (Available.x < 1.0f)
                {
                    Available.x = 1.0f;
                }
                if (Available.y < 1.0f)
                {
                    Available.y = 1.0f;
                }

                ImVec2 ImageMin = ImGui::GetCursorScreenPos();
                ImGui::Image(
                    UIRenderer::GetTextureID(UIRenderer::ViewportTextureSlot),
                    Available,
                    ImVec2(0.0f, 0.0f),
                    ImVec2(1.0f, 1.0f)
                );
                ImVec2 ImageMax = ImVec2(ImageMin.x + Available.x, ImageMin.y + Available.y);

                mViewportHovered = ImGui::IsItemHovered();
                mViewportSize = Available;

                ImDrawList* DrawList = ImGui::GetWindowDrawList();
                DrawList->AddRect(ImageMin, ImageMax,
                    ImGui::GetColorU32(mViewportHovered ? Accent : Border), 4.0f, 0, mViewportHovered ? 2.0f : 1.0f);

                const char* OverlayText = mViewportHovered ? "Viewport focused" : "Scene preview";
                ImVec2 OverlayPos = ImVec2(ImageMin.x + 12.0f, ImageMin.y + 12.0f);
                DrawList->AddRectFilled(
                    ImVec2(OverlayPos.x - 8.0f, OverlayPos.y - 5.0f),
                    ImVec2(OverlayPos.x + 126.0f, OverlayPos.y + 20.0f),
                    ImGui::GetColorU32(ImVec4(0.030f, 0.036f, 0.045f, 0.780f)),
                    4.0f
                );
                DrawList->AddText(OverlayPos, ImGui::GetColorU32(Text), OverlayText);
            }
            ImGui::EndChild();

            ImGui::SameLine(0.0f, Gutter);

            ImGui::BeginChild("InspectorPanel", ImVec2(RightPanelWidth, 0.0f), true);
            {
                DrawSectionHeader("Inspector");

                if (mActiveDemoIndex >= 0 && mActiveDemoIndex < static_cast<int16_t>(mRenderDemos.size()))
                {
                    ImGui::TextDisabled("Active Layer");
                    ImGui::TextUnformatted(mRenderDemos[mActiveDemoIndex]->GetName().c_str());
                }
                else
                {
                    ImGui::TextDisabled("No active layer");
                }
                DrawSectionHeader("Demo Settings");
                if (IRenderDemo* Demo = mRenderDemos[mActiveDemoIndex].get())
                {
                    Demo->OnRenderUI();
                }
                DrawSectionHeader("Viewport");
                ImGui::TextDisabled("Size");
                ImGui::SameLine(120.0f);
                ImGui::Text("%.0f x %.0f", mViewportSize.x, mViewportSize.y);

                ImGui::TextDisabled("Hovered");
                ImGui::SameLine(120.0f);
                ImGui::TextColored(mViewportHovered ? Success : TextMuted, "%s", mViewportHovered ? "Yes" : "No");

                DrawSectionHeader("Renderer");
                ImGui::TextDisabled("Backend");
                ImGui::SameLine(120.0f);
                ImGui::TextUnformatted("DirectX 12");

                ImGui::TextDisabled("Frame");
                ImGui::SameLine(120.0f);
                ImGui::Text("%.3f ms", 1000.0f / (ImGui::GetIO().Framerate > 0.0f ? ImGui::GetIO().Framerate : 1.0f));

                DrawSectionHeader("Selection");
                ImGui::TextDisabled("Nothing selected");
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + Gutter);
        ImGui::BeginChild("StatusBar", ImVec2(ContentSize.x - Gutter * 2.0f, StatusBarHeight), false, ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::TextColored(Success, "Ready");
            ImGui::SameLine();
            ImGui::TextDisabled("| DX12 | RenderGraph | Viewport %.0f x %.0f", mViewportSize.x, mViewportSize.y);

            const char* RightText = "Lumina Editor";
            float TextWidth = ImGui::CalcTextSize(RightText).x;
            float RightX = ImGui::GetWindowWidth() - TextWidth - 8.0f;
            if (RightX > ImGui::GetCursorPosX())
            {
                ImGui::SameLine(RightX);
                ImGui::TextDisabled("%s", RightText);
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleColor(ColorCount);
    ImGui::PopStyleVar(StyleVarCount);
}
