#include "Logger/Logger.h"

#include <cstdint>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

#include <fstream>
#include <iostream>

#define MAX_CONSOLE_LINES 1000

namespace Log
{
    constexpr const char* LuminaDefaultLogFilePath = "LuminaLog.txt";
    static std::ofstream OutFileStream;

    void SetConsoleColor(WORD Color)
    {
        HANDLE ConsoleOuput = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(ConsoleOuput, Color);
    }

    void InitConsole()
    {
        // Create a console for current application
        AllocConsole();

        // Get Standard Output Handle
        HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        int32_t SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
        std::FILE* COutputHandle = _fdopen(SystemOutput, "w");

        // Get Standard Error Handle
        HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
        int32_t SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
        std::FILE* CErrorHandle = _fdopen(SystemError, "w");

        // Get Standard Input Handle
        HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
        int32_t SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
        std::FILE* CInputHandle = _fdopen(SystemInput, "r");

        // Redirect cout, wcout, cin, wcin, wcerr and cerr
        std::ios::sync_with_stdio(true);

        // Redirect the CRT standard input, output, error handles
        freopen_s(&CInputHandle, "CONIN$", "r", stdin);
        freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
        freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

        // Clear Error State
        std::wcout.clear();
        std::cout.clear();
        std::wcerr.clear();
        std::cerr.clear();
        std::wcin.clear();
        std::cin.clear();
    }

    void Initialize(OutputMode OutputMode, std::string_view LogFilePath)
    {
        InitConsole();
        SetConsoleColor(15);
    }

    void Destroy()
    {
        // TODO: Get Current Time
        std::string Message = "[Log] Exit...";
        if (OutFileStream.is_open())
        {
            OutFileStream << Message;
            OutFileStream.close();
        }
        std::cout << Message;
        OutputDebugStringA(Message.c_str());
    }

    void Error(std::string_view Category, std::string_view Message)
    {
        std::string Error = "[Error]\t[";
        Error += Category;
        Error += "]\t: ";
        Error += Message;
        Error += "\n";

        OutputDebugStringA(Error.c_str());
        if (OutFileStream.is_open())
        {
            OutFileStream << Error;
        }
        SetConsoleColor(12);
        std::cout << Error;
        SetConsoleColor(15);
    }

    void Info(std::string_view Category, std::string_view Message)
    {
        std::string Info = "[Info]\t[";
        Info += Category;
        Info += "]\t: ";
        Info += Message;
        Info += "\n";

        OutputDebugStringA(Info.c_str());
        if (OutFileStream.is_open())
        {
            OutFileStream << Info;
        }
        SetConsoleColor(15);
        std::cout << Info;
    }

    void Warning(std::string_view Category, std::string_view Message)
    {
        std::string Warning = "[Warning]\t[";
        Warning += Category;
        Warning += "]\t: ";
        Warning += Message;
        Warning += "\n";

        OutputDebugStringA(Warning.c_str());
        if (OutFileStream.is_open())
        {
            OutFileStream << Warning;
        }
        SetConsoleColor(14);
        std::cout << Warning;
        SetConsoleColor(15);
    }
}