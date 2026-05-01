#include <memory>

#include "Engine/LuminaEngine.h"
#include "Engine/Input.h"
#include "Logger/Logger.h"
#include "Renderer/Renderer.h"

bool LuminaEngine::Initialize(FStartupParameters& StartupParameters, LuminaApp* Application)
{
    mCurrentApp = Application;

    InitializeWindows(StartupParameters);

    Renderer::Initialize(mpMainWindow->GetHWND(), mpMainWindow->GetWidth(), mpMainWindow->GetHeight());

    Input::Init();

    mCurrentApp->InitializeApp(mpMainWindow->GetHWND(), mpMainWindow->GetWidth(), mpMainWindow->GetHeight());

    return true;
}

void LuminaEngine::Run()
{
    mbIsRunning = true;
    mTimer.Reset();
    const double FIXED_DT = 1.0 / 60.0;
    double Accumulator = 0.0;

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
        if (!mbIsRunning) break;

        mTimer.Tick();
        double DeltaTime = mTimer.GetDeltaTime();
        if (DeltaTime > 0.25) DeltaTime = 0.25;

        Accumulator += DeltaTime;
        while (Accumulator >= FIXED_DT)
        {
            mCurrentApp->FixedUpdateApp(FIXED_DT);
            Accumulator -= FIXED_DT;
        }

        mCurrentApp->UpdateApp(DeltaTime);

        FCommandContext* pMainContext = Renderer::BeginFrame();
        Input::Update();
        mCurrentApp->RenderApp(pMainContext);

        Renderer::EndFrame(pMainContext);
    }

    mCurrentApp->DestroyApp();

    Renderer::Shutdown();

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