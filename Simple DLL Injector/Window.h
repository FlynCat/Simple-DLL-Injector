#pragma once
#include <Windows.h>
#include <string>
namespace Window
{
    HWND Create(const std::string& title, const std::string& class_name, HINSTANCE hIns = 0);
    HWND GetHwnd();
    DWORD GetProcessId();
    void DropFile(const std::string& path);
    void Destroy();
    bool PumpMsg();
};

