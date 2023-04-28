#include "Ui.h"
#include "State.h"
#include "Util.h"
#include "Logger.h"
#include "Window.h"
#include <string>
#include <imgui.h>

using namespace std;

namespace ui {

    void DrawProcessCombo(const char* lastProcess, ProcessInfo& currentProcess) {
        auto& processList = util::GetProcessList();
        if (ImGui::BeginCombo("##process_combo", state::selectedProcess)) // The second parameter is the label previewed before opening the combo.
        {
            for (auto n = 0u; n < processList.size(); n++)
            {
                auto generateId = [](ProcessInfo& process) {
                    return process.name + "##" + to_string((int)process.hwnd);
                };
                auto id = generateId(processList[n]);
                auto currentId = generateId(currentProcess);
                bool is_selected = (id == currentId); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(id.c_str(), is_selected)) {
                    state::selectedProcess = processList[n].name.c_str();
                    state::processIdx = n;
                }
                if (ImGui::IsItemHovered()) {
                    auto utf = util::WideToUTF8(processList[n].title);
                    ImGui::SetTooltip("%s", utf.c_str());
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", currentProcess.fullPath.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            util::RefreshProcessList();
            for (size_t i{ 0 }; i < processList.size(); i++) {
                auto& process = processList[i];
                if (process.name == lastProcess) {
                    state::processIdx = i;
                    break;
                }
            }
        }
    }

    void DrawDllListbox() {
        if (ImGui::BeginListBox("##dll_list"))
        {
            for (size_t n = 0; n < state::dlls.size(); n++)
            {
                const bool is_selected = (state::dllIdx == n);
                auto fileExists = state::dlls[n].exists;
                if (ImGui::Selectable(state::dlls[n].name.c_str(), is_selected, fileExists ? 0 : ImGuiSelectableFlags_Disabled)) {
                    state::dllIdx = n;
                }
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::Button("Remove?")) {
                        state::dlls.erase(state::dlls.begin() + n);
                        while (state::dllIdx > 0 && state::dllIdx >= state::dlls.size()) {
                            state::dllIdx--;
                        }
                        state::save();
                        ImGui::CloseCurrentPopup();
                        n--; // decrement n by 1 to account for the removed element
                    }
                    ImGui::EndPopup();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    if (fileExists)
                        ImGui::SetTooltip("%s", state::dlls[n].full.c_str());
                    else
                        ImGui::SetTooltip("[X] File does not exists! [X]");
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        if (!state::dlls.empty()) {
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
                ImGui::OpenPopup("Clear?");
        }
        if (ImGui::BeginPopupModal("Clear?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("The DLL List will be cleared\nContinue?");
            if (ImGui::Button("Clear All", ImVec2(120, 0))) {
                state::dlls.clear();
                state::save();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear not exists", ImVec2(120, 0))) {
                //state::dlls.clear();
                state::dlls.erase(
                    std::remove_if(
                        state::dlls.begin(),
                        state::dlls.end(),
                        [](const DllFile& dll) { return !dll.exists; }),
                    state::dlls.end());
                state::save();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_Escape)) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }
    }

    void DrawPreInfo(ProcessInfo& currentProcess) {
        ImGui::Text("%-10s : %s", "Process", currentProcess.name.c_str());
        if (ImGui::IsItemHovered()) {
            auto utf = util::WideToUTF8(currentProcess.title);
            ImGui::SetTooltip("%s", utf.c_str());
        }
        if (!state::dlls.empty()) {
            ImGui::Text("%-10s : %s", "DLL", state::getCurrentDll().name.c_str());
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", state::getCurrentDll().full.c_str());
            }
        }
    }

    bool autoInject = false;
    float secAutoInject = 1.f;
    float msCountdown = .0f;
    void Draw() {
        static auto& io = ImGui::GetIO();
        const char* lastProcess = state::lastProcess.c_str();
        if (!state::dlls.empty() && !state::getCurrentDll().lastProcess.empty())
            lastProcess = state::getCurrentDll().lastProcess.c_str();
        ImGui::Begin("Main Window", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("Process List | Last Process : %s", lastProcess);               // Display some text (you can use a format strings too)
        //ImGui::ShowDemoWindow();
        auto& processList = util::GetProcessList();
        auto currentProcess = processList.at(state::processIdx);
        state::selectedProcess = currentProcess.name.c_str();
        DrawProcessCombo(lastProcess, currentProcess);
        DrawDllListbox();
        DrawPreInfo(currentProcess);

        //TODO: refactor needed
        if (!state::dlls.empty()) {
            if (ImGui::Button("Inject", { ImGui::GetContentRegionAvail().x,40 })) {
                if (util::CheckProcessModule(currentProcess.id, state::getCurrentDll().name.c_str())) {
                    //TODO: request confirmation
                    LOG_INFO("%s already loaded in the process!", state::getCurrentDll().name.c_str());
                }
                else {
                    //util::Inject(currentProcess.id, state::getCurrentDll().full);
                    if (util::Inject(currentProcess.id, state::getCurrentDll().full)) {
                        LOG_INFO("%s Injected to %s", state::getCurrentDll().name.c_str(), currentProcess.name.c_str());
                    }
                    else {
                        LOG_ERROR("%s Injection failed into %s", state::getCurrentDll().name.c_str(), currentProcess.name.c_str());
                    }
                    state::getCurrentDll().lastProcess = currentProcess.name;
                    state::save();
                }
            }
            //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); // Set the item spacing to zero
            static float aotWidth = 0.f;
            if (state::getCurrentDll().lastProcess != "") {

                if (ImGui::Checkbox("Auto", &autoInject)) {
                    if (autoInject) {
                        LOG_DEBUG("Auto Inject enabled");
                    }
                    else {
                        LOG_DEBUG("Auto Inject disabled");
                        msCountdown = .0f;
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Automatically inject to Last Process");
                }
                ImGui::SameLine();
            }
            static bool topMost = false;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - aotWidth));
            if (ImGui::Checkbox("Always On Top", &topMost)) {
                auto hwnd = Window::GetHwnd();
                if (topMost) {
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                else {
                    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Keep window on top");
            }
            aotWidth = ImGui::GetItemRectSize().x;

            if (autoInject) {
                ImGui::DragFloat("second(s)", &secAutoInject, 0.1f, 0.1f, 10.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                if (currentProcess.name == lastProcess && msCountdown <= .0f) {
                    msCountdown = secAutoInject;
                }
                else if (currentProcess.name != lastProcess) {
                    msCountdown = 0.0f;
                }
                if (msCountdown > 0.0f) {
                    msCountdown -= io.DeltaTime;
                }
                if (msCountdown <= 0.0f && currentProcess.name == lastProcess) {
                    autoInject = false;
                    msCountdown = .0f;
                    if (util::CheckProcessModule(currentProcess.id, state::getCurrentDll().name.c_str())) {
                        //TODO: request confirmation
                        LOG_INFO("[AUTO] %s already loaded in the process!", state::getCurrentDll().name.c_str());
                    }
                    else {
                        if (util::Inject(currentProcess.id, state::getCurrentDll().full)) {
                            LOG_INFO("[AUTO] %s Injected to %s", state::getCurrentDll().name.c_str(), currentProcess.name.c_str());
                        }
                        else {
                            LOG_ERROR("[AUTO] %s Injection failed into %s", state::getCurrentDll().name.c_str(), currentProcess.name.c_str());
                        }
                        state::save();
                    }
                }
                if (msCountdown > 0.0) {
                    ImGui::SameLine();
                    ImGui::Text("[Injecting in %d]", (int)ceil(msCountdown));
                }
            }
        }
        ImGui::Separator();
        logger::Draw("Log");

        ImGui::End();
    }
}