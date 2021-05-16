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
#include <stdarg.h>

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
std::wstring Get_Full_Path(DWORD pID);
std::wstring Get_Full_Path(HWND window);
std::wstring Name_From_Path(std::wstring path);

//Proc.cpp
#define PT_PAUSE  0
#define PT_RESUME 1
#define PT_KILL   2

void Process_All_Window(HWND window, int ActionID = PT_KILL, int killReturnValue = -1);
bool Process_All(std::wstring process_name, bool isPath, int ActionID = PT_KILL, int killReturnValue = -1);

bool Process(DWORD pID, int ActionID = PT_KILL, int killReturnValue = -1);
bool Process_Tree(DWORD pID, int ActionID = PT_KILL, int killReturnValue = -1, HANDLE hSnap = nullptr, VectorEx <DWORD>* pid_vec = nullptr);
void Process_Only(HWND window, int ActionID = PT_KILL, int killReturnValue = -1);

//main
struct JimmyGlobalProps_t
{
    std::atomic <bool> Media_Overlay_Service;
    std::atomic <bool> Locker_Service;
    std::atomic <DWORD> Locker_ActivateKey;
    std::atomic <DWORD> Locker_ExitKey;
    std::atomic <bool> Locker_IsLocked;

    std::atomic < bool> Block_Injected_Mouse;
    std::atomic < bool> Block_Injected_Keyboard;
};

#define WSTR(Input) std::wstring(CA2W(Input, CP_UTF8))
#define STR(Input) std::string(CW2A(Input, CP_UTF8))

extern JimmyGlobalProps_t Jimmy_Global_properties;

LRESULT CALLBACK JimmyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam, VectorEx<key_deskriptor> keydesk, bool repeated_input);
LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);


//json parsers
class target_process_t
{
public:
    enum class process_type_t { path, name };
    process_type_t Trg_tp;
    std::wstring process;
};

class target_window_t
{
public:
    enum class target_window_type_t { current_window, undermouse_window };
    target_window_type_t Trg_tp;

    HWND get();
};

class process_Action_t
{
    target_window_t wind;
    target_process_t proc;

public:
    enum class Act_Type_t { killAll, pauseAll, killOnly, pauseOnly, resumeAll, resumeOnly };
    enum class Trg_type_t { window, process };
    enum class area_t { all, only };

    Act_Type_t Act_Type;
    Trg_type_t Trg_type;

    void Set_Target(target_window_t wnd);
    void Set_Target(target_process_t prc);

    void Perform();
};

class window_Action_t
{
    target_window_t target;

public:
    enum class Act_Type_t { minimize, topmost, noTopmost };

    Act_Type_t Act_Type;

    void Set_Target(target_window_t wnd);

    void Perform();

};

class toggle_input_blocking_t
{
public:

    enum class target_t { mouse, keyboard, all };
    enum class allowment_t { injectedOnly, hardwareOnly, All };
    enum class Action_t { toggle, set };

    target_t    Trg;
    allowment_t All;
    Action_t    Act;
    bool Target_Val;

    void Perform();
};

class Action_desk
{
public:
    enum class Action_type_t { process, window, input };

protected:

    process_Action_t p;
    toggle_input_blocking_t i;
    window_Action_t w;

    Action_type_t atype;

public:

    size_t Uuid;
    void setAction(process_Action_t Act);

    void setAction(window_Action_t Act);

    void setAction(toggle_input_blocking_t Act);

    void Perform();
};


int Get_HKPP_Constant_From_String(char* str);
Action_desk Get_Action_Object(cJSON* command);
bool Load_Config(DWORD tID = GetCurrentThreadId());

extern VectorEx <Action_desk> Actions;

//Log.cpp
void Log(const char* Log_str, ...);


void Locker_immproc();