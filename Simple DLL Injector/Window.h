#pragma once
#include <Windows.h>
#include <string>
namespace Window
{
    HWND Create(const std::string& title, const std::string& class_name, HINSTANCE hIns = 0);
    HWND GetHwnd();
    DWORD GetProcessId();
    const char* GetTitle();
    void DropFile(const std::string& path);
    void Destroy();
    bool PumpMsg();
};

