#pragma once
#include <string>
#include <windows.h>
#include <tlhelp32.h>

#define PT_PAUSE  0
#define PT_RESUME 1
#define PT_KILL   2

void KillAll(HWND window, int actionID = PT_KILL, int killReturnValue = -1);
void KillAll(std::wstring process_name, bool isPath, int actionID = PT_KILL, int killReturnValue = -1);
bool Process(DWORD pID, int actionID, int killReturnValue);
bool Process_Tree(DWORD pID, int actionID = PT_KILL, int killReturnValue = -1, HANDLE hSnap = NULL);