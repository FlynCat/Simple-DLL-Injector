#include "DirectX.h"
#include "Window.h"
#include <imgui.h>
#include <vector>
#include <string>
#include <Psapi.h>

using namespace std;

struct ProcessInfo {
	string name;
	string title;
	DWORD id;
	HWND hwnd;
};

std::vector<ProcessInfo> processList;
size_t selectedIndex = 0;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
	//DWORD dwThreadId;
	//HINSTANCE hInstance;
	char winTitle[255];
	if (!hWnd)															return TRUE;        // Not a window
	if (!::IsWindowVisible(hWnd))										return TRUE;        // Not visible
	if (!SendMessage(hWnd, WM_GETTEXT, sizeof(winTitle), (LPARAM)winTitle))	return TRUE;        // No window title
	//if (hWnd == GetConsoleWindow())										return TRUE;		// Not our console window

	//hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	///*dwThreadId = */
	DWORD procId;
	GetWindowThreadProcessId(hWnd, &procId);
	char processName[MAX_PATH];
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
	GetModuleFileNameExA(handle, 0, processName, MAX_PATH);
	// Extract the file name from the path
	char* name = strrchr(processName, '\\');
	if (name != NULL) {
		name++; // Move past the backslash
	}
	else {
		name = processName;
	}
	//wstring winT(winTitle);
	processList.push_back({ string(name),string(winTitle),procId,hWnd });
	CloseHandle(handle);
	//pID.push_back(procId);
	//process.push_back(to_hex(procId) + "\t" + winT);

	//cout << "PID:" << hex << uppercase << procId  << '\t' << String << '\t' << endl;
	return TRUE;
}

void RefreshProcess() {
	processList.clear();
	EnumWindows(EnumWindowsProc, 0);
}

void DirectX::Render()
{
	auto& io = ImGui::GetIO();
	ImGui::ShowDemoWindow();
	ImGui::Begin("Main Window", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);                          // Create a window called "Hello, world!" and append into it.
	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	static const char* current_item = processList.at(selectedIndex).name.c_str();
	if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
	{
		for (auto n = 0u; n < processList.size(); n++)
		{
			bool is_selected = (current_item == processList[n].name); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(processList[n].name.c_str(), is_selected)) {
				current_item = processList[n].name.c_str();
				selectedIndex = n;
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
		}
		ImGui::EndCombo();
	}
	ImGui::Text("%s", processList.at(selectedIndex).title.c_str());
	ImGui::End();
}


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) {
	HWND hwnd = Window::Create("Simple DLL Injector", "simple_dll_injector");
	// Initialize Direct3D
	if (!DirectX::Init(hwnd))
	{
		DirectX::Destroy();
		Window::Destroy();
		return 1;
	}
	DirectX::ImGuiInit();
	// Main loop
	RefreshProcess();
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
