#pragma once

#include "hotkeyPP.h"
using namespace HKPP;
using namespace HKPP::extra;

#include "cJSON.h"

#include <string>
#include <windows.h>
#include <atlbase.h>
#include <atlconv.h>
#include <ShObjIdl.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <tlhelp32.h>

//Media_Tools.cpp
float Get_Volume();
bool Set_Volume(float new_bolume);
void Disable_Overlay();
void Enable_Overlay();

//Sys_Init.cpp
bool Is_Running_As_Admin();
void Sys_Init(int argc, char** argv);

//Keyboard_Convertors
DWORD StrToKey(std::wstring key_str);
std::wstring keyToStr(DWORD key);

//Path.cpp
std::wstring GetFullPath(DWORD pID);
std::wstring GetFullPath(HWND window);
std::wstring NameFromPath(std::wstring path);

//Proc.cpp
#define PT_PAUSE  0
#define PT_RESUME 1
#define PT_KILL   2

void ProcessAll(HWND window, int actionID = PT_KILL, int killReturnValue = -1);
void ProcessAll(std::wstring process_name, bool isPath, int actionID = PT_KILL, int killReturnValue = -1);
bool Process(DWORD pID, int actionID = PT_KILL, int killReturnValue = -1);
bool ProcessTree(DWORD pID, int actionID = PT_KILL, int killReturnValue = -1, HANDLE hSnap = NULL);
void ProcessOnly(HWND window, int actionID = PT_KILL, int killReturnValue = -1);


struct JimmyGlobalProps_t
{
    bool MediaOverlayServiceEnabled;
    bool LockerServiceEnabled;
    bool BlockInjected_Mouse;
    bool BlockInjected_Keyboard;
};

#define WSTR(Input) std::wstring(CA2W(Input, CP_UTF8))
#define STR(Input) std::string(CW2A(Input, CP_UTF8))

extern std::atomic<JimmyGlobalProps_t> Jimmy_Global_properties;

LRESULT CALLBACK JimmyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

int GetHKPP_ConstantFromString(char* str);
void StandartHotkeyHandler(std::wstring command_str);
void standartCommandParcer(cJSON* command);
bool LoadConfig();