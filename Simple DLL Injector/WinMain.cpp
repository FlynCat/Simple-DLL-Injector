#include "DirectX.h"
#include "Window.h"
#include <imgui.h>
#include "State.h"
#include "Util.h"
#include <algorithm>
#include <ranges>
using namespace std;


void Window::DropFile(const std::string& file) {
    if (!util::isFileDll(file)) return;
    auto name = strrchr(file.c_str(), '\\');
    auto name2 = name + 1;
    //bool found = std::ranges::any_of(files, [&file](const auto& dll) {
    //    return dll.full == file;
    //    });
    auto found = std::ranges::any_of(state::dlls, [&file](const auto& dll) {return dll.full == file; });
    if (!found) {
        //files.push_back(file);
        state::dlls.push_back({ name2,file });
    }
    state::save();
}


void DirectX::Render()
{
    auto& io = ImGui::GetIO();
    static bool injected = false;
    const char* lastProcess = state::lastProcess.c_str();
    if (!state::dlls.empty() && !state::dlls[state::dllIdx].lastProcess.empty())
        lastProcess = state::dlls[state::dllIdx].lastProcess.c_str();
    ImGui::Begin("Main Window", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);                          // Create a window called "Hello, world!" and append into it.
    ImGui::Text("Process List | Last Process : %s", lastProcess);               // Display some text (you can use a format strings too)
    //ImGui::ShowDemoWindow();
    auto& processList = util::GetProcessList();
    auto currentProcess = processList.at(state::processIdx);
    state::selectedProcess = currentProcess.name.c_str();
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
            if (process.name.compare(lastProcess) == 0) {
                state::processIdx = i;
                break;
            }
        }
    }
    if (ImGui::BeginListBox("##dll_list"))
    {
        for (size_t n = 0; n < state::dlls.size(); n++)
        {
            const bool is_selected = (state::dllIdx == n);
            if (ImGui::Selectable(state::dlls[n].name.c_str(), is_selected)) {
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
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", state::dlls[n].full.c_str());
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
    if (ImGui::BeginPopupModal("Clear?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("The DLL List will be cleared\nContinue?");
        if (ImGui::Button("Yes", ImVec2(120, 0))) {
            state::dlls.clear();
            state::save();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }



    ImGui::Text("%-10s : %s", "Process", currentProcess.name.c_str());
    if (ImGui::IsItemHovered()) {
        auto utf = util::WideToUTF8(currentProcess.title);
        ImGui::SetTooltip("%s", utf.c_str());
    }
    if (!state::dlls.empty()) {
        static bool autoInject = false;
        ImGui::Text("%-10s : %s", "DLL", state::dlls[state::dllIdx].name.c_str());
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", state::dlls[state::dllIdx].full.c_str());
        }
        if (ImGui::Button("Inject")) {
            util::Inject(currentProcess.id, state::dlls[state::dllIdx].full);
            state::dlls[state::dllIdx].lastProcess = currentProcess.name;
            state::save();
        }
        //TODO: use ms to delay injection
        static float ms = 1.f;
        if (!ImGui::Checkbox("Auto", &autoInject)) {
            injected = false;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Automatically inject to Last Process");
        }
        if (autoInject) {
            ImGui::DragFloat("ms", &ms, 0.1f, 0.1f, 10.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        }
        if (autoInject && currentProcess.name == lastProcess && !injected) {
            injected = true;
            util::Inject(currentProcess.id, state::dlls[state::dllIdx].full);
            state::save();
        }
        if (injected)
            ImGui::Text("Injected");
    }

    ImGui::End();
}
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime) {
    if (event == EVENT_SYSTEM_FOREGROUND && !state::dlls.empty()) {
        // The foreground window has changed, do something here
        auto& processList = util::RefreshProcessList();
        for (size_t i{ 0 }; i < processList.size(); i++) {
            auto& process = processList[i];
            if (process.name == state::dlls[state::dllIdx].lastProcess) {
                state::processIdx = i;
                break;
            }
        }
    }
}
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) {  // Set the hook
    auto hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, HandleWinEvent, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (hook == NULL) {
        // Failed to set hook
        auto err = GetLastError();
        return 1;
    }
    HWND hwnd = Window::Create("Simple DLL Injector", "simple_dll_injector", hInstance);
    // Initialize Direct3D
    if (!DirectX::Init(hwnd))
    {
        DirectX::Destroy();
        Window::Destroy();
        return 1;
    }
    DirectX::ImGuiInit();

    util::RefreshProcessList();
    state::load();
    auto lastHwnd = hwnd;
    // Main loop
    while (Window::PumpMsg())
    {
        DirectX::Begin();
        DirectX::Render();
        DirectX::End();
    }
    DirectX::ImGuiDestroy();
    // Cleanup
    Window::Destroy();
    DirectX::Destroy();

    // Unhook when done
    UnhookWinEvent(hook);

    return 0;
}
