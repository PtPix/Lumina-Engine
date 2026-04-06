#include <windows.h>

#include "StringUtils/StringConv.h"

std::string StringUtils::WideToUTF8(const wchar_t* wideStr)
{
    if (wideStr == nullptr) return "";

    const int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0) return "";

    std::string utf8Str(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &utf8Str[0], bufferSize, nullptr, nullptr);

    utf8Str.resize(bufferSize - 1);
    return utf8Str;
}

std::string StringUtils::WideToUTF8(std::wstring_view wideStr)
{
    if (wideStr.empty()) return "";

    const int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), static_cast<int>(wideStr.length()), nullptr, 0, nullptr, nullptr);
    if (bufferSize <= 0) return "";

    std::string utf8Str(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), static_cast<int>(wideStr.length()), &utf8Str[0], bufferSize, nullptr, nullptr);

    return utf8Str;
}

std::wstring StringUtils::UTF8ToWide(const char* utf8Str)
{
    if (utf8Str == nullptr) return L"";

    const int bufferSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);
    if (bufferSize <= 0) return L"";

    std::wstring wideStr(bufferSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &wideStr[0], bufferSize);

    wideStr.resize(bufferSize - 1);
    return wideStr;
}

std::wstring StringUtils::UTF8ToWide(std::string_view utf8Str)
{
    if (utf8Str.empty()) return L"";

    const int bufferSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.length()), nullptr, 0);
    if (bufferSize <= 0) return L"";

    std::wstring wideStr(bufferSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.length()), &wideStr[0], bufferSize);

    return wideStr;
}
