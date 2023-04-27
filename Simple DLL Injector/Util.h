#pragma once
#include <string>
#include <vector>
#include <Windows.h>
struct ProcessInfo {
    std::string name; //.exe
    std::string fullPath;
    std::wstring title;
    DWORD id;
    HWND hwnd;
};

namespace util
{
    bool isFileDll(const std::string& path);
    std::string WideToUTF8(const std::wstring& wstr);
    std::wstring UTF8ToWide(const std::string& str);
    std::string GetTime();
    std::vector<ProcessInfo>& GetProcessList();
    std::vector<ProcessInfo>& RefreshProcessList();
    DWORD GetProcessId(const char* processName);
    std::vector<std::string> GetProcessModules(DWORD pid);
    bool CheckProcessModule(DWORD pid, const char* module);
    bool Inject(DWORD processId, const std::string& dll);

};

