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
    HINSTANCE hExeInstance{};
    int iCommandShow{};

    int Width = -1;
    int Height = -1;
    EDisplayMode DisplayMode = EDisplayMode::WINDOWED;
    int PreferredDisplay = 0;
    wchar_t* Title = new wchar_t[64];
};

struct FStartupParameters
{
    FWindowSettings MainWindowSettings;
    FGraphicsSettings Graphics;
};