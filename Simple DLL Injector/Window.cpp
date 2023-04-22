#include "Window.h"
#include "Util.h"
#include <imgui.h>
#include "DirectX.h"
#include <codecvt>
#include <locale>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace Window {
    HWND hWnd;
    WNDCLASSEXW wc;

    // This function takes in a wParam from the WM_DROPFILES message and 
    // prints all the files to a message box.
    void HandleFiles(WPARAM wParam)
    {
        // DragQueryFile() takes a LPWSTR for the name so we need a TCHAR string
        TCHAR szName[MAX_PATH];

        // Here we cast the wParam as a HDROP handle to pass into the next functions
        HDROP hDrop = (HDROP)wParam;

        // This functions has a couple functionalities.  If you pass in 0xFFFFFFFF in
        // the second parameter then it returns the count of how many filers were drag
        // and dropped.  Otherwise, the function fills in the szName string array with
        // the current file being queried.
        int count = DragQueryFile(hDrop, 0xFFFFFFFF, szName, MAX_PATH);

        // Here we go through all the files that were drag and dropped then display them
        for (int i = 0; i < count; i++)
        {
            // Grab the name of the file associated with index "i" in the list of files dropped.
            // Be sure you know that the name is attached to the FULL path of the file.
            DragQueryFile(hDrop, i, szName, MAX_PATH);

            // Bring up a message box that displays the current file being processed
            Window::DropFile(std::string(szName));
        }

        // Finally, we destroy the HDROP handle so the extra memory
        // allocated by the application is released.
        DragFinish(hDrop);
    }
    // Forward declare message handler from imgui_impl_win32.cpp

    // Win32 message handler
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                DirectX::HandleResize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED:
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
            {
                //const int dpi = HIWORD(wParam);
                //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
                const RECT* suggested_rect = (RECT*)lParam;
                ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            break;
        case WM_DROPFILES:
            HandleFiles(wParam);
            break;
        }

        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }


    HWND Create(const std::string& title, const std::string& class_name)
    {
        // Convert title string to wide-character string
        std::wstring wtitle = util::UTF8ToWide(title);
        std::wstring wclass = util::UTF8ToWide(class_name);
        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, wclass.c_str(), nullptr };
        ::RegisterClassExW(&wc);
        HWND hwnd = CreateWindowExW(WS_EX_ACCEPTFILES, wc.lpszClassName, wtitle.c_str(), WS_OVERLAPPEDWINDOW, 100, 100, 500, 350, nullptr, nullptr, wc.hInstance, nullptr);
        Window::wc = wc;
        Window::hWnd = hwnd;

        // Show the window
        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);
        return hwnd;
    }
    HWND GetHwnd()
    {
        return hWnd;
    }
    void Destroy()
    {
        ::DestroyWindow(Window::hWnd);
        ::UnregisterClassW(Window::wc.lpszClassName, wc.hInstance);
    }
    bool PumpMsg()
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                return false;
        }


        return true;
    }
}
