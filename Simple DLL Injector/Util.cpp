#include "Util.h"
#include <Windows.h>
#include <ranges>
#include <Psapi.h>
#include <algorithm>
#include "Window.h"
#include "logger.h"
using namespace std;
namespace util {
    bool isFileDll(const std::string& path) {
        auto dotIdx = path.find_last_of('.');
        if (dotIdx == string::npos) return false;
        auto ext = path.substr(dotIdx, path.length() - dotIdx);
        auto extIt = ext | ranges::views::transform([](char c) { return std::tolower(c); });
        string extLower = string(extIt.begin(), extIt.end());
        if (extLower != ".dll") {
            return false;
        }
        return true;
    }

    std::wstring UTF8ToWide(const std::string& str)
    {
        int wtitle_length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
        std::wstring wtitle(wtitle_length, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wtitle[0], wtitle_length);
        return wtitle;
    }

#pragma region PROCESS STUFF
    std::vector<ProcessInfo> processList;
    std::vector<ProcessInfo>& GetProcessList()
    {
        if (processList.empty()) RefreshProcessList();
        return processList;
    }

    BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
        //char winTitle[MAX_PATH];
        auto titleLen = GetWindowTextLength(hWnd);
        if (!hWnd)															return TRUE;        // Not a window
        //if (hWnd == (HWND)lParam)											return TRUE;		// Not our window
        if (!::IsWindowVisible(hWnd))										return TRUE;        // Not visible
        if (!titleLen)														return TRUE;		// No window title
        titleLen++;
        //if (!SendMessage(hWnd, WM_GETTEXT, sizeof(winTitle), (LPARAM)winTitle))	return TRUE;        // No window title
        //if (hWnd == GetConsoleWindow())										return TRUE;		// Not our console window
        wchar_t* title = new wchar_t[titleLen];
        GetWindowTextW(hWnd, title, titleLen);
        auto windowTitle = wstring(title);
        delete[] title;
        //hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
        ///*dwThreadId = */
        DWORD procId;
        GetWindowThreadProcessId(hWnd, &procId);
        if (procId == DWORD(lParam)) return TRUE;
        auto pidExist = std::ranges::any_of(
            processList,
            [&procId, &lParam](const auto& process) {
                return process.id == procId;
            });
        if (pidExist) return TRUE;

        char processName[MAX_PATH];
        HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, procId);
        GetModuleFileNameExA(handle, 0, processName, MAX_PATH);

        if (!handle) {
            auto err = GetLastError();
            LOG_ERROR("Failed To Get Process Handle : (%x)", err);
        }

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
    bool Inject(DWORD processId, const std::string& dll) {
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (!handle) {
            LOG_ERROR("Invalid handle");
            return false;
        }
        void* loc = VirtualAllocEx(handle, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!loc) {
            LOG_ERROR("Fail to allocate memory");
            return false;
        }
        if (!WriteProcessMemory(handle, loc, dll.c_str(), dll.length() + 1, 0)) {
            LOG_ERROR("Fail to write dll file");
            return false;
        }
        HANDLE hThread = CreateRemoteThread(handle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
        if (!hThread) {
            LOG_ERROR("CreateRemoteThread failed");
            return false;
        }
        CloseHandle(handle);
        CloseHandle(hThread);
        return true;
    }
#pragma endregion

    std::string WideToUTF8(const std::wstring& wstr)
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        std::string utf8str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8str.data(), size_needed, NULL, NULL);
        return utf8str;
    }

}
