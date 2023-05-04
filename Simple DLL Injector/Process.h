#pragma once
#include <vector>
#include <string>
#include <Windows.h>
struct ProcessInfo {
    std::string name; //.exe
    std::string fullPath;
    std::wstring title;
    DWORD id;
    HWND hwnd;
};
namespace process
{
    std::vector<ProcessInfo>& GetProcessList();
    std::vector<ProcessInfo>& RefreshProcessList();
    DWORD GetProcessId(const char* processName);
    std::vector<std::string> GetProcessModules(DWORD pid);
    bool CheckProcessModule(DWORD pid, const char* module);
    DWORD CallRemoteThread(HANDLE handle, void* func, LPVOID param);
    bool Inject(DWORD processId, const std::string& dll);
    std::string ErrorCodeToString(DWORD errorMessageID);
};

