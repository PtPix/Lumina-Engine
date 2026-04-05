#include "Renderer/RenderCore/Shader.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>

FShaderMacro FShaderMacro::CreateShaderMacro(const char* Name, const char* Format, ...)
{
    FShaderMacro Macro{};

    strncpy_s(Macro.Name, Name, sizeof(Macro.Name) - 1);
    Macro.Name[sizeof(Macro.Name) - 1] = '\0';

    va_list args;
    va_start(args, Format);
    vsnprintf(Macro.Value, sizeof(Macro.Value), Format, args);
    va_end(args);

    return Macro;
}
