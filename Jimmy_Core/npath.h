#pragma once
#include <string>
#include <windows.h>

std::wstring GetFullPath(DWORD pID);
std::wstring GetFullPath(HWND window);
std::wstring NameFromPath(std::wstring path);