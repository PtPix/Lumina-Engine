#pragma once

#include <string>
#include <string_view>

namespace StringUtils
{
    [[nodiscard]] std::string WideToUTF8(const wchar_t* wideStr);
    [[nodiscard]] std::string WideToUTF8(std::wstring_view wideStr);
    [[nodiscard]] std::wstring UTF8ToWide(const char* utf8Str);
    [[nodiscard]] std::wstring UTF8ToWide(std::string_view utf8Str);
}