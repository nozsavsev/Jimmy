#pragma once
#include <windows.h>
#include <string>
DWORD StrToKey(std::wstring key_str);
std::wstring keyToStr(DWORD key);