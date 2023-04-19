#pragma once
#include <Windows.h>
namespace DirectX
{
	bool Init(HWND hWnd);
	void Destroy();
	void ImGuiInit();
	void ImGuiDestroy();
	void HandleResize(UINT newWidth, UINT newHeight);
	void Begin();
	void Render();
	void End();
};

