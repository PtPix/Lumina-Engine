#include "Logger/Logger.h"

#include "Engine/Settings.h"
#include "Engine/LuminaEngine.h"

#include "Samples/ModelApp.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR pScmdl, int iCmdShow)
{
    // Set Start Parameters
    FStartupParameters StartupParameters = {};
    StartupParameters.hExeInstance = hInst;
    StartupParameters.iCommandShow = iCmdShow;
    StartupParameters.EngineSettings.WindowMain.Width = 1280;
    StartupParameters.EngineSettings.WindowMain.Height = 720;
    wcscpy_s(StartupParameters.EngineSettings.WindowMain.Title, 64, L"Lumina Window");

    // Init Engine
    LuminaEngine Engine = {};
    ModelApp ModelApp = {};
    if (!Engine.Initialize(StartupParameters, &ModelApp))
    {
        LUMINA_LOG_ERROR(Main, "Failed to initialize Engine.");
        return -1;
    }

    Engine.Run();

    Engine.Quit();

    return 0;
}
