#include "Process.h"
#include <Psapi.h>
#include <algorithm>
#include <WtsApi32.h>
#include "Window.h"
#include "Logger.h"
#include "Util.h"
#include <TlHelp32.h>

#pragma comment(lib,"Wtsapi32.lib")

using namespace std;

namespace process {

    std::vector<ProcessInfo> processList;
    std::vector<ProcessInfo>& GetProcessList()
    {
        if (processList.empty()) RefreshProcessList();
        return processList;
    }

    BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
        auto titleLen = GetWindowTextLength(hWnd);
        //=======Basic Checks=============
        if (!titleLen)                                              return TRUE;		// No window title
        if (!hWnd)                                                  return TRUE;        // Not a window
        DWORD procId;
        GetWindowThreadProcessId(hWnd, &procId);
        if (!::IsWindowVisible(hWnd) && procId != (DWORD)lParam)    return TRUE;        // Not visible
        //================================
        //=======Get Window Title========//
        titleLen++;
        wchar_t* title = new wchar_t[titleLen];
        GetWindowTextW(hWnd, title, titleLen);
        auto windowTitle = wstring(title);
        delete[] title;
        //===============================//
        //========Check any duplicate====//
        auto pidExist = std::ranges::any_of(
            processList,
            [&procId, &lParam](const auto& process) {
                return process.id == procId;
            });
        if (pidExist) return TRUE;
        //===============================//

        char processName[MAX_PATH];
        HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, procId);
        GetModuleFileNameExA(handle, 0, processName, MAX_PATH);

        //if (!handle) {
        //    auto err = GetLastError();
        //    LOG_ERROR("Failed To Get Process Handle from PID (%d) : (%s)", procId, ErrorCodeToString(err).c_str());
        //}

        static auto compatibleArch = [](HANDLE hProcess)
        {
            BOOL isTargetWow64 = FALSE;
            if (!IsWow64Process(hProcess, &isTargetWow64)) {
                LOG_DEBUG("IsWow64Process failed!");
            }
            BOOL isInjectorWow64 = FALSE;
            if (!IsWow64Process(GetCurrentProcess(), &isInjectorWow64)) {
                LOG_DEBUG("IsWow64Process failed!");
            }
            if (isTargetWow64 == isInjectorWow64) {
                return true;
            }
            return false;
        };
        if (!compatibleArch(handle)) return TRUE;

        // Extract the file name from the path
        char* name = strrchr(processName, '\\');
        if (name != NULL) {
            name++; // Move past the backslash
        }
        else {
            name = processName;
        }
        processList.push_back({ string(name),string(processName),windowTitle,procId,hWnd });
        if (handle)
            CloseHandle(handle);
        return TRUE;
    }
    vector<ProcessInfo>& RefreshProcessList()
    {
        processList.clear();
        DWORD procId = 0x0;
        GetWindowThreadProcessId(Window::GetHwnd(), &procId);
        EnumWindows(EnumWindowsProc, LPARAM(procId));
        return processList;
    }

    DWORD CallRemoteThread(HANDLE handle, void* func, LPVOID param) {
        HANDLE hThread = CreateRemoteThread(handle, 0, 0, (LPTHREAD_START_ROUTINE)func, param, 0, 0);
        if (!hThread) {
            auto error = ErrorCodeToString(GetLastError());
            LOG_ERROR("CreateRemoteThread failed. (%s)", error.c_str());
            return NULL;
        }
        WaitForSingleObject(hThread, INFINITE);
        DWORD result = NULL;
        GetExitCodeThread(hThread, &result);
        CloseHandle(hThread);
        return (DWORD)result;
    }


    bool Inject(DWORD processId, const std::string& dll) {
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        auto dllw = util::UTF8ToWide(dll);
        if (!handle) {
            auto error = ErrorCodeToString(GetLastError());
            LOG_ERROR("Invalid handle. (%s)", error.c_str());
            return false;
        }
        void* loc = VirtualAllocEx(handle, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!loc) {
            auto error = ErrorCodeToString(GetLastError());
            LOG_ERROR("Failed to allocate memory. (%s)", error.c_str());
            CloseHandle(handle);
            return false;
        }
        if (!WriteProcessMemory(handle, loc, dllw.c_str(), (dllw.length() + 1) * sizeof(wchar_t), 0)) {
            auto error = ErrorCodeToString(GetLastError());
            LOG_ERROR("Failed to write dll file to target process. (%s)", error.c_str());
            CloseHandle(handle);
            return false;
        }

        auto hModule = CallRemoteThread(handle, LoadLibraryW, loc);

        bool result = true;
        if (hModule == NULL) {
            auto errorCode = CallRemoteThread(handle, GetLastError, 0);
            auto error = ErrorCodeToString(errorCode);
            LOG_ERROR("LoadLibrary Error on target process! (%s)", error.c_str());
            result = false;
        }

        if (loc) VirtualFreeEx(handle, loc, 0, MEM_RELEASE);
        CloseHandle(handle);
        return result;
    }
    DWORD GetProcessId(const char* processName) {
        DWORD pCount;
        PWTS_PROCESS_INFO pInfos;
        DWORD procId = 0;
        if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &pInfos, &pCount)) {
            for (DWORD i = 0; i < pCount; i++) {
                auto& p = pInfos[i];
                if (strcmp(p.pProcessName, processName) == 0) {
                    procId = p.ProcessId;
                    break;
                }

                // do something with the process ID
            }

            WTSFreeMemory(pInfos);
        }
        return procId;

    }

    std::vector<std::string> GetProcessModules(DWORD pid)
    {
        std::vector<std::string> modules;

        MODULEENTRY32 me32;

        auto hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (hModuleSnap == INVALID_HANDLE_VALUE)
            return modules;

        me32.dwSize = sizeof(MODULEENTRY32);

        if (!Module32First(hModuleSnap, &me32))
        {
            CloseHandle(hModuleSnap);
            return modules;
        }

        do
            modules.push_back(me32.szModule);
        while (Module32Next(hModuleSnap, &me32));

        CloseHandle(hModuleSnap);
        return modules;
    }
    bool CheckProcessModule(DWORD pid, const char* module) {
        MODULEENTRY32 me32;

        auto hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (hModuleSnap == INVALID_HANDLE_VALUE) {
            LOG_ERROR("Fail to check process module : Invalid handle!");
            return false;
        }

        me32.dwSize = sizeof(MODULEENTRY32);

        if (!Module32First(hModuleSnap, &me32))
        {
            LOG_ERROR("Fail to get process module!");
            CloseHandle(hModuleSnap);
            return false;
        }
        bool found = false;
        do
            if (strcmp(me32.szModule, module) == 0) {
                found = true;
                break;
            }
        while (Module32Next(hModuleSnap, &me32));
        CloseHandle(hModuleSnap);
        return found;
    }
    //https://stackoverflow.com/a/17387176
    //Returns the last Win32 error, in string format. Returns an empty string if there is no error.
    std::string ErrorCodeToString(DWORD errorMessageID)
    {
        //Get the error message ID, if any.
        //DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0) {
            return std::string(); //No error message has been recorded
        }

        LPSTR messageBuffer = nullptr;

        //Ask Win32 to give us the string version of that message ID.
        //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        //Copy the error message into a std::string.
        std::string message(messageBuffer, size);

        //Free the Win32's string's buffer.
        LocalFree(messageBuffer);

        return message.erase(message.find_last_not_of(" \n\r\t") + 1);;
    }
}
