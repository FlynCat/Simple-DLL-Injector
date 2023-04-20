#pragma once
#include <Windows.h>
#include <string>
namespace Window
{
	HWND Create(const std::string& title, const std::string& class_name);
	void Destroy();
	bool PumpMsg();
};

