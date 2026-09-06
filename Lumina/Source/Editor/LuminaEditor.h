#pragma once

#include <memory>
#include <vector>

#include "IRenderDemo.h"
#include "Engine/LuminaApp.h"
#include "ImGUI/imgui.h"

class LuminaEditor : public LuminaApp
{
public:
    LuminaEditor() = default;
    ~LuminaEditor() override = default;

    template<typename T>
    void RegisterTestLayer()
    {
        static_assert(std::is_base_of_v<IRenderDemo, T>, "T must inherit from IRenderDemo");
        mRenderDemos.push_back(std::make_unique<T>());
    }

protected:
    bool OnInit() override;
    void OnUpdate(double DeltaTime) override;
    void OnFixedUpdate(double FixedDeltaTime) override;
    void OnRender(FRenderGraph& RenderGraph) override;
    void OnRenderUI(FRenderGraph& RenderGraph) override;
    void OnDestroy() override;

    void SetActiveLayer(int16_t ActiveLayer);

private:
    void RenderEditorUI(FRenderGraph& RenderGraph);

    std::vector<std::unique_ptr<IRenderDemo>> mRenderDemos;
    int16_t mActiveDemoIndex = -1;

    ImVec2 mViewportSize = ImVec2(0, 0);
    bool mViewportHovered = false;
};
