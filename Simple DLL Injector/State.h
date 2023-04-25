#pragma once
#include <vector>
#include <string>
struct DllFile {
    std::string name;
    std::string full;
    std::string lastProcess;
};

namespace state {
    inline size_t processIdx = 0;
    inline size_t dllIdx = 0;
    inline const char* selectedProcess;
    inline std::string lastProcess;
    inline std::vector<DllFile> dlls;
    void save();
    void load();
}

