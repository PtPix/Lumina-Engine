#include <memory>

#include "Engine/LuminaEngine.h"
#include "Engine/Input.h"
#include "Logger/Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/UI/UIRenderer.h"

bool FLuminaEngine::Initialize(const FStartupParameters& StartupParameters, LuminaApp* Application)
{
    if (!Application)
    {
        LUMINA_LOG_ERROR(Engine, "Initialize failed: Application is null.");
        return false;
    }

    mWindowSettings = StartupParameters.MainWindowSettings;
    mGraphicsSettings = StartupParameters.Graphics;
    mCurrentApp = Application;

    if (!InitMainWindow())
    {
        LUMINA_LOG_ERROR(Engine, "MainWindow Initialize failed.");
        return false;
    }

    if (!InitRenderer())
    {
        LUMINA_LOG_ERROR(Engine, "Renderer Initialize failed.");
        return false;
    }

    if (!InitInput())
    {
        LUMINA_LOG_ERROR(Engine, "Input Initialize failed.");
        return false;
    }

    if (!InitApplication())
    {
        LUMINA_LOG_ERROR(Engine, "Application Initialize failed.");
        return false;
    }

    mbInitialized = true;
    return true;
}

void FLuminaEngine::Run()
{
    if (!mbInitialized) return;

    mbRunning = true;
    mTimer.Reset();

    while (mbRunning)
    {
        if (!PumpPlatformMessages()) break;

        Tick();
    }

    Shutdown();
}

void FLuminaEngine::RequestQuit()
{
    mbRunning = false;
}

bool FLuminaEngine::InitMainWindow()
{
    FWindowDesc WindowDesc = {};
    WindowDesc.Width = mWindowSettings.Width;
    WindowDesc.Height = mWindowSettings.Height;
    WindowDesc.bFullScreen = mWindowSettings.DisplayMode == EDisplayMode::EXCLUSIVE_FULLSCREEN;
    WindowDesc.PreferredDisplay = mWindowSettings.PreferredDisplay;
    WindowDesc.iShowCommand = mWindowSettings.iCommandShow;
    WindowDesc.hInstance = mWindowSettings.hExeInstance;

    WindowDesc.Callbacks.fnNativeMessageHandler = [](const FWin32Message& Message, LRESULT& OutResult)
    {
        return UIRenderer::ProcessWin32Message(Message.Hwnd, Message.Message, Message.WParam, Message.LParam, OutResult);
    };
    WindowDesc.Callbacks.fnOnDestroy = [this](FWindow&) { RequestQuit(); };
    WindowDesc.Callbacks.fnOnResize = [](FWindow&, uint32_t Width, uint32_t Height)
    {
        if (Width == 0 || Height == 0) return;
        Renderer::OnResize(Width, Height);
    };

    mpMainWindow = std::make_unique<FWindow>(mWindowSettings.Title, WindowDesc);
    return mpMainWindow != nullptr;
}

bool FLuminaEngine::InitRenderer()
{
    return Renderer::Initialize(mpMainWindow->GetHWND(), mpMainWindow->GetWidth(), mpMainWindow->GetHeight());
}

bool FLuminaEngine::InitInput()
{
    return Input::Init();
}

bool FLuminaEngine::InitApplication()
{
    return mCurrentApp->InitializeApp(mpMainWindow->GetHWND(), mpMainWindow->GetWidth(), mpMainWindow->GetHeight());
}

void FLuminaEngine::Tick()
{
    constexpr double FixedDeltaTime = 1.0 / 60.0;
    constexpr double MaxDeltaTime = 0.25;

    mTimer.Tick();

    double DeltaTime = mTimer.GetDeltaTime();
    if (DeltaTime > MaxDeltaTime)
    {
        DeltaTime = MaxDeltaTime;
    }

    mFixedAccumulator += DeltaTime;
    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (mFixedAccumulator >= FixedDeltaTime)
    {
        mCurrentApp->FixedUpdateApp(FixedDeltaTime);
        mFixedAccumulator -= FixedDeltaTime;
    }

    mCurrentApp->UpdateApp(DeltaTime);

    Renderer::BeginFrame();
    mCurrentApp->RenderApp(Renderer::GetRenderGraph());
    Renderer::EndFrame();

    Input::Update();
}

void FLuminaEngine::Shutdown()
{
    if (mCurrentApp)
    {
        mCurrentApp->DestroyApp();
        mCurrentApp = nullptr;
    }

    Renderer::Shutdown();
    mpMainWindow.reset();

    mbInitialized = false;
}

bool FLuminaEngine::PumpPlatformMessages()
{
    MSG Message = {};

    while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
    {
        if (Message.message == WM_QUIT)
        {
            RequestQuit();
            return false;
        }

        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return mbRunning;
}
