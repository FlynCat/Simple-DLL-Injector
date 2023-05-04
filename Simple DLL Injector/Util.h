#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace util
{
    bool isFileDll(const std::string& path);
    std::string WideToUTF8(const std::wstring& wstr);
    std::wstring UTF8ToWide(const std::string& str);
    std::string GetTime();

};

