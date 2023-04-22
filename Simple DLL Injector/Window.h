#pragma once
#include <Windows.h>
#include <string>
namespace Window
{
	HWND Create(const std::string& title, const std::string& class_name);
	HWND GetHwnd();
	void DropFile(const std::string& path);
	void Destroy();
	bool PumpMsg();
};

