#include "Util.h"
#include <Windows.h>
#include <ranges>
#include <Psapi.h>
#include <algorithm>
#include <WtsApi32.h>
#include "Window.h"
#include "Logger.h"
#include <TlHelp32.h>
#include <chrono>

#pragma comment(lib,"Wtsapi32.lib")

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



    std::string WideToUTF8(const std::wstring& wstr)
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        std::string utf8str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8str.data(), size_needed, NULL, NULL);
        return utf8str;
    }
    std::string GetTime() {
        // Get the current system time
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        // Convert the time to the local time
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
        localtime_s(&now_tm, &now_c);

        // Format the time string in the desired format
        std::string timeString = std::format("{:02d}:{:02d}:{:02d}", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);

        return timeString;
    }

}
