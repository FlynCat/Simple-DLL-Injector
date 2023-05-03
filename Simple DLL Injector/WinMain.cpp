#include "DirectX.h"
#include "Window.h"
#include "State.h"
#include "Util.h"
#include "Logger.h"


void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime) {
    if (event == EVENT_SYSTEM_FOREGROUND && !state::dlls.empty()) {
        // The foreground window has changed, do something here
        auto& processList = util::RefreshProcessList();
        for (size_t i{ 0 }; i < processList.size(); i++) {
            auto& process = processList[i];
            if (process.name == state::getCurrentDll().lastProcess) {
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
    logger::Clear();
    auto hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, HandleWinEvent, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (hook == NULL) {
        // Failed to set hook
        auto err = GetLastError();
        return 1;
    }
#ifdef _WIN64
    HWND hwnd = Window::Create("Simple DLL Injector - x64", "simple_dll_injector_x64", hInstance);
#else
    HWND hwnd = Window::Create("Simple DLL Injector", "simple_dll_injector", hInstance);
#endif // _WIN64

    if (!hwnd) {
        UnhookWinEvent(hook);
        return 1;
    }
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
