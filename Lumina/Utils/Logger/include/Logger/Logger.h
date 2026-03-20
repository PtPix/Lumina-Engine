#pragma once

#include <string_view>
#include <cstdio>

#define LUMINA_LOG_FUNCTION(FUNCTION_NAME)\
template<class... Args>\
void FUNCTION_NAME(const char* Format, Args&&... args)\
{\
    char Message[LEN_MESSAGE_BUFFER];\
    sprintf_s(Message, Format, args...);\
    FUNCTION_NAME(std::string_view(Message));\
}

namespace Log
{
    enum class OutputMode : unsigned
    {
        None    = 0,
        Console = 1 << 0,
        File    = 1 << 1,

        ConsoleAndFile = Console | File,
    };

    inline OutputMode operator|(OutputMode A, OutputMode B)
    {
        return static_cast<OutputMode>(static_cast<unsigned>(A) | static_cast<unsigned>(B));
    }

    // --------------------------------------------

    constexpr size_t LEN_MESSAGE_BUFFER = 4096;

    // --------------------------------------------

    // TODO: Only Console for now.
    void Initialize(OutputMode OutputMode = OutputMode::None, std::string_view LogFilePath = "");

    void Destroy();

    void Info(std::string_view Message);
    void Warning(std::string_view Message);
    void Error(std::string_view Message);

    LUMINA_LOG_FUNCTION(Info)
    LUMINA_LOG_FUNCTION(Warning)
    LUMINA_LOG_FUNCTION(Error)
}