#pragma once
#include <Windows.h>
#include <string>
namespace Window
{
	HWND Create(const std::wstring & title, const std::wstring & class_name);
	void Destroy();
	bool PumpMsg();
};

