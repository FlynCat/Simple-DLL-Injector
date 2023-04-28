#pragma once
#include <Windows.h>
namespace DirectX
{
    bool Init(HWND hWnd);
    void Destroy();
    void ImGuiInit();
    void ImGuiDestroy();
    void SetResize(UINT newWidth, UINT newHeight);
    void HandleResize();
    void Begin();
    void Render();
    void End();
};

