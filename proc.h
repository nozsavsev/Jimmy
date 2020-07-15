#ifndef PROC_H
#define PROC_H

#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <mutex>
#include <vector>

#define PT_PAUSE  0
#define PT_RESUME 1
#define PT_KILL   2


bool Process(DWORD pID, int actionID, int killReturnValue);

bool Process_Tree(DWORD pID, int actionID, int killReturnValue = 1);

extern std::mutex          pid_protected_mutex;
extern std::vector <DWORD> pid_protected;

#endif//PROC_H