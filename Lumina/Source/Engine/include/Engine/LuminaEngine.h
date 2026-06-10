#pragma once

#include "LuminaApp.h"
#include "Profiler/Profiler.h"

#include "Settings.h"
#include "Platform/Window.h"

#include <memory>

class FLuminaEngine
{
public:
    FLuminaEngine() = default;

    bool Initialize(const FStartupParameters& StartupParameters, LuminaApp* Application);
    void Run();
    void RequestQuit();

private:
    bool InitMainWindow();
    bool InitRenderer();
    bool InitInput();
    bool InitApplication();

    void Tick();
    void Shutdown();

    bool PumpPlatformMessages();

    std::unique_ptr<FWindow> mpMainWindow;
    LuminaApp* mCurrentApp{};

    FrameTimer mTimer;
    double mFixedAccumulator = 0.0;

    bool mbRunning = false;
    bool mbInitialized = false;

    // Settings
    FWindowSettings mWindowSettings;
    FGraphicsSettings mGraphicsSettings;
};

