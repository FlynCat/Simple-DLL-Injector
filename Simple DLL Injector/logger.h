#pragma once

//pasted straight from the imgui_demo.cpp
namespace logger
{

    void    Clear();

    void    AddLog(const char* fmt, ...);

    void    Draw(const char* title, bool* p_open = 0);
};

#define LOG_INFO(fmt, ...) logger::AddLog("[%s] " fmt "\n",util::GetTime().c_str(), ##__VA_ARGS__)
#ifdef _DEBUG
#define LOG_DEBUG(fmt, ...) logger::AddLog("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) 
#endif
#define LOG_ERROR(fmt, ...) logger::AddLog("[ERROR] " fmt "\n", ##__VA_ARGS__)

