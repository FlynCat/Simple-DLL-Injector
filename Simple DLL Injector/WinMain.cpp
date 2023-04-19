#include "DirectX.h"
#include <imgui.h>
#include "Window.h"
#include <tchar.h>

void DirectX::Render()
{
	auto& io = ImGui::GetIO();
	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Hello, world!", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) {
	HWND hwnd = Window::Create(L"WWWW", L"simple_dll_injector");
	// Initialize Direct3D
	if (!DirectX::Init(hwnd))
	{
		DirectX::Destroy();
		Window::Destroy();
		return 1;
	}
	DirectX::ImGuiInit();
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

	return 0;
}
