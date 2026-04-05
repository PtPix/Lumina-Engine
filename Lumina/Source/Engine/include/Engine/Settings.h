#pragma once

#include <windows.h>

enum EDisplayMode
{
    WINDOWED = 0,
    BORDERLESS_FULLSCREEN,
    EXCLUSIVE_FULLSCREEN,

    NUM_DISPLAY_MODES
};

struct FGraphicsSettings
{

};

struct FWindowSettings
{
    int Width = -1;
    int Height = -1;
    EDisplayMode DisplayMode = EDisplayMode::WINDOWED;
    int PreferredDisplay = 0;
    wchar_t* Title = new wchar_t[64];
};

struct FEngineSettings
{
    FGraphicsSettings Graphics;

    FWindowSettings WindowMain;
};

struct LogInitializeParameters
{
    bool bLogConsole = false;
    bool bLogFile = false;
    char LogFilePath[512]{};
};

struct FStartupParameters
{
    HINSTANCE hExeInstance{};
    int iCommandShow{};
    LogInitializeParameters LogInitParameters;

    FEngineSettings EngineSettings;
};