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
std::wstring GetFullPath(DWORD pID);
std::wstring GetFullPath(HWND window);
std::wstring NameFromPath(std::wstring path);

//Proc.cpp
#define PT_PAUSE  0
#define PT_RESUME 1
#define PT_KILL   2

void ProcessAll_Window(HWND window, int actionID = PT_KILL, int killReturnValue = -1);
bool ProcessAll(std::wstring process_name, bool isPath, int actionID = PT_KILL, int killReturnValue = -1);

bool Process(DWORD pID, int actionID = PT_KILL, int killReturnValue = -1);
bool ProcessTree(DWORD pID, int actionID = PT_KILL, int killReturnValue = -1, HANDLE hSnap = nullptr, VectorEx <DWORD>* pid_vec = nullptr);
void ProcessOnly(HWND window, int actionID = PT_KILL, int killReturnValue = -1);

//main
struct JimmyGlobalProps_t
{
    std::atomic < bool> MediaOverlayService;
    std::atomic < bool> LockerService;

    std::atomic < bool> BlockInjected_Mouse;
    std::atomic < bool> BlockInjected_Keyboard;
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
    process_type_t trg_tp;
    std::wstring process;
};

class target_window_t
{
public:
    enum class target_window_type_t { current_window, undermouse_window };
    target_window_type_t trg_tp;

    HWND get();
};

class process_action_t
{
    target_window_t wind;
    target_process_t proc;

public:
    enum class act_type_t { killAll, pauseAll, killOnly, pauseOnly, resumeAll, resumeOnly };
    enum class trg_type_t { window, process };
    enum class area_t { all, only };

    act_type_t act_type;
    trg_type_t trg_type;

    void set_target(target_window_t wnd);
    void set_target(target_process_t prc);

    void perform();
};

class window_action_t
{
    target_window_t target;

public:
    enum class act_type_t { minimize, topmost, noTopmost };

    act_type_t act_type;

    void set_target(target_window_t wnd);

    void perform();

};

class toggle_input_blocking_t
{
public:

    enum class target_t { mouse, keyboard, all };
    enum class allowment_t { injectedOnly, hardwareOnly, All };
    enum class action_t { toggle, set };

    target_t    trg;
    allowment_t all;
    action_t    act;
    bool targetVal;

    void perform();
};

class action_desk
{
public:
    enum class action_type_t { process, window, input };

protected:

    process_action_t p;
    toggle_input_blocking_t i;
    window_action_t w;

    action_type_t atype;

public:

    size_t uuid;
    void setAction(process_action_t act);

    void setAction(window_action_t act);

    void setAction(toggle_input_blocking_t act);

    void perform();
};


int GetHKPP_ConstantFromString(char* str);
action_desk GetActionObject(cJSON* command);
bool LoadConfig(DWORD tID = GetCurrentThreadId());

extern VectorEx <action_desk> actions;

//log.cpp
void log(const char* log_str, ...);