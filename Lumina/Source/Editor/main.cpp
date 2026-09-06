#include "Logger/Logger.h"
#include "Engine/Settings.h"
#include "Engine/LuminaEngine.h"
#include "LuminaEditor.h"

#include <memory>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR pScmdl, int iCmdShow)
{
    // Set Start Parameters
    FStartupParameters StartupParameters = {};
    StartupParameters.MainWindowSettings.hExeInstance = hInst;
    StartupParameters.MainWindowSettings.iCommandShow = iCmdShow;
    StartupParameters.MainWindowSettings.Width = 1280;
    StartupParameters.MainWindowSettings.Height = 720;
    wcscpy_s(StartupParameters.MainWindowSettings.Title, 64, L"Lumina Window");

    // Init Engine
    auto Engine = std::make_unique<FLuminaEngine>();
    auto App = std::make_unique<LuminaEditor>();

    if (!Engine->Initialize(StartupParameters, App.get()))
    {
        LUMINA_LOG_ERROR(Main, "Failed to initialize Engine.");
        return -1;
    }

    // Run Engine
    Engine->Run();
    Engine->RequestQuit();

    return 0;
}
