#pragma once

#include <memory>
#include <vector>

#include "Engine/LuminaApp.h"
#include "ImGUI/imgui.h"
#include "Renderer/RenderGraph/RenderGraph.h"
#include "TestLayer/ITestLayer.h"

struct FViewportRenderTarget
{
    FRGTextureHandle Color;
    FRGTextureHandle Depth;
    uint32_t Width = 1;
    uint32_t Height = 1;
};

class LuminaEditor : public LuminaApp
{
public:
    LuminaEditor() = default;
    ~LuminaEditor() override = default;

    template<typename T>
    void RegisterTestLayer()
    {
        static_assert(std::is_base_of_v<ITestLayer, T>, "T must inherit from ITestLayer");
        mTestLayers.push_back(std::make_unique<T>());
    }

protected:
    bool OnInit() override;
    void OnUpdate(double DeltaTime) override;
    void OnFixedUpdate(double FixedDeltaTime) override;
    void OnRender(FRenderGraph& RenderGraph) override;
    void OnRenderUI(FRenderGraph& RenderGraph) override;
    void OnDestroy() override;

private:
    void RenderEditorUI(FRenderGraph& RenderGraph);

    std::vector<std::unique_ptr<ITestLayer>> mTestLayers;
    int16_t mActiveLayerIndex = -1;

    ImVec2 mViewportSize = ImVec2(0, 0);
    bool mViewportHovered = false;
};
