#pragma once

#include <memory>
#include <vector>

#include "Engine/LuminaApp.h"
#include "TestLayer/ITestLayer.h"

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
    void OnRender(FCommandContext* pCommandContext) override;
    void OnRenderUI(FCommandContext* pCommandContext) override;
    void OnDestroy() override;

private:
    void RenderEditorUI();

    std::vector<std::unique_ptr<ITestLayer>> mTestLayers;
    int16_t mActiveLayerIndex = -1;
};
