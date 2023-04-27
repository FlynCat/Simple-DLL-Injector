#pragma once

//pasted straight from the imgui_demo.cpp
namespace logger
{

    void    Clear();

    void    AddLog(const char* fmt, ...);

    void    Draw(const char* title, bool* p_open = 0);
};

#define LOG_INFO(fmt, ...) logger::AddLog("[%.1f] " fmt "\n",ImGui::GetTime(), ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) logger::AddLog("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger::AddLog("[ERROR] " fmt "\n", ##__VA_ARGS__)

