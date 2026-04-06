#include "Engine/LuminaEngine.h"
#include "Logger/Logger.h"

#include <memory>

#include "Engine/Input.h"

bool LuminaEngine::Initialize(FStartupParameters& StartupParameters, LuminaApp* Application)
{
    mCurrentApp = Application;
    InitializeWindows(StartupParameters);

    if (!mGraphicsDevice.Initialize(mpMainWindow->GetHWND(), mpMainWindow->GetWidth(), mpMainWindow->GetHeight()))
    {
        LUMINA_LOG_ERROR(Engine, "Can't Initialize Graphics Device.");
        return false;
    }

    if (!mCurrentApp)
    {
        LUMINA_LOG_ERROR(Engine, "Can't Run Engine without App.");
        return false;
    }

    mCurrentApp->InitializeApp(&mGraphicsDevice, mpMainWindow->GetHWND(), mpMainWindow->GetWidth(), mpMainWindow->GetHeight());

    return true;
}

void LuminaEngine::Run()
{
    mbIsRunning = true;
    mTimer.Reset();
    Input::Init();

    MSG msg = {};
    while (mbIsRunning)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                mbIsRunning = false;
                break;
            }
        }
        if (!mbIsRunning)
        {
            break;
        }

        mTimer.Tick();

        mCurrentApp->UpdateApp(mTimer.GetDeltaTime());
        mCurrentApp->RenderApp();

        Input::Update();

        if (mTimer.UpdateAndCheckReportInterval())
        {
            LUMINA_LOG_INFO(Engine, "FPS: %.2f, Avg Frame: %.3fms, Total: %.1fs",
                   mTimer.GetFPS(), mTimer.GetAvgFrameTimeMs(), mTimer.GetTotalTime());
        }
    }

    mCurrentApp->DestroyApp();
    mGraphicsDevice.Destroy();
    if (mpMainWindow)
    {
        mpMainWindow.reset();
    }
}

void LuminaEngine::Quit()
{
    mbIsRunning = false;
}


void LuminaEngine::InitializeWindows(FStartupParameters& StartupParameters)
{
    FWindowSettings WindowSettings = StartupParameters.EngineSettings.WindowMain;
    FWindowDesc WindowDesc = {};
    WindowDesc.Width = WindowSettings.Width;
    WindowDesc.Height = WindowSettings.Height;
    WindowDesc.hInstance = StartupParameters.hExeInstance;
    WindowDesc.pWindowOwner = static_cast<IWindowOwner*>(this);
    WindowDesc.pfnWindowProcedure = WindowProc;
    WindowDesc.bFullScreen = WindowSettings.DisplayMode == EDisplayMode::EXCLUSIVE_FULLSCREEN;
    WindowDesc.PreferredDisplay = WindowSettings.PreferredDisplay;
    WindowDesc.iShowCommand = StartupParameters.iCommandShow;
    WindowDesc.WindowName = "Lumina Window";
    WindowDesc.OnRegisterWindowName = [this](HWND hwnd, const std::string& Name) {
        this->SetWindowName(hwnd, Name);
    };
    mpMainWindow = std::make_unique<Window>(WindowSettings.Title, WindowDesc);
    mpMainWindow->pOwner->OnWindowCreate(mpMainWindow->GetHWND());
    LUMINA_LOG_INFO(Engine, "Created main window<%p>: %dx%d", mpMainWindow->GetHWND(), mpMainWindow->GetWidth(), mpMainWindow->GetHeight());
}
