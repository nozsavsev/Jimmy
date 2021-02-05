#include <windows.h>
#include <tlhelp32.h>

#define _CRT_SECURE_NO_WARNINGS

#include "hotkeyPP.h"

using namespace HKPP;
using namespace HKPP::extra;

#define PT_PAUSE  0
#define PT_RESUME 1
#define PT_KILL   2

bool Is_Running_As_Admin();
void Sys_Init(int argc, char** argv);
void KillAll(HWND window, int actionID = PT_KILL, int killReturnValue = -1);
void KillAll(std::wstring process_name, int actionID = PT_KILL, int killReturnValue = -1);
bool Process(DWORD pID, int actionID, int killReturnValue);
bool Process_Tree(DWORD pID, int actionID = PT_KILL, int killReturnValue = -1, HANDLE hSnap = NULL);
std::wstring GetFullPath(DWORD pID);
std::wstring GetFullPath(HWND window);


class pid_vector : protected std::vector <DWORD>, protected std::mutex
{
public:
    DWORD operator += (DWORD pid)
    {
        lock();
        this->push_back(pid);
        unlock();
        return pid;
    }

    bool operator == (DWORD pid)
    {
        for (DWORD p_pid : *this)
            if (p_pid == pid)
                return true;
        return false;
    }

    bool operator != (DWORD pid)
    {
        return !(operator == (pid));
    }

    bool operator [] (DWORD pid)
    {
        return (operator == (pid));
    }
} static pvec;


int main(int argc, char** argv)
{
    {
        DWORD pID = 0;
        GetWindowThreadProcessId(GetConsoleWindow(), &pID);

        pvec += GetCurrentProcessId();
        pvec += pID;
    }

    Sys_Init(argc, argv);

    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();
    mng->HKPP_Init();

    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU , 'T' },
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_ALLOW_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"kill All Instances of current window",
            [&](void) -> void { KillAll(GetForegroundWindow()); })
    ));

    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU ,VK_LSHIFT, 'T' },
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_ALLOW_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"kill All Instances of undermose window",
            [&](void) -> void
            {
                POINT P;
                GetCursorPos(&P);
                KillAll(WindowFromPoint(P));
            })
    ));

    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        if (msg.message == WM_HKPP_DEFAULT_CALLBACK_MESSAGE)
        {
            if (msg.lParam)
            {
                Hotkey_Deskriptor* dsk = (Hotkey_Deskriptor*)msg.lParam;
                wprintf(L"\"%s\" %s\n", dsk->settings.name.c_str(), dsk->Real ? L"Real" : L"Injected");
                dsk->settings.user_callback();
                delete dsk;
            }
        }

        DispatchMessageW(&msg);
    }

    mng->HKPP_Stop();

    return 0;
}

bool Is_Running_As_Admin()
{
    BOOL is_run_as_admin = FALSE;
    PSID administrators_group = NULL;
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administrators_group))
    {
        FreeSid(administrators_group);
        return false;
    }

    else
        CheckTokenMembership(NULL, administrators_group, &is_run_as_admin);

    FreeSid(administrators_group);

    return is_run_as_admin ? true : false;
}

void Sys_Init(int argc, char** argv)
{
    printf("starting JIMMY_V3 |> Vista1nik edition\n");
    if (!Is_Running_As_Admin())
    {
        MessageBoxW(NULL, L"jimmy cannot work without administrator privileges\npress Ok to restart winth admin rights", L"jimmy - accessibility alert", MB_OK | MB_TOPMOST);
        {
            wchar_t szPath[MAX_PATH];

            GetModuleFileNameW(NULL, szPath, MAX_PATH);

            SHELLEXECUTEINFO sei = { sizeof(sei) };
            sei.lpParameters = L"-restarted_by_enother_instance";
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;

            if (ShellExecuteEx(&sei))
                exit(0);
        }
    }

    CreateMutex(NULL, true, L"JYMMY_GLOBAL_START_MUTEX");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxW(NULL, L"jimmy is alaredy running!", L"jimmy alert", MB_OK | MB_TOPMOST);
        exit(0);
    }
}

std::wstring GetFullPath(HWND window)
{
    DWORD pID = 0;
    GetWindowThreadProcessId(window, &pID);
    return GetFullPath(pID);
}

std::wstring GetFullPath(DWORD pID)
{
    std::wstring name = L"";
    DWORD buffSize = 1024;
    wchar_t buffer[1024];

    HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pID);

    if (handle)
    {
        if (QueryFullProcessImageNameW(handle, 0, buffer, &buffSize))
            name = buffer;
        CloseHandle(handle);
    }

    return name;
}

std::wstring NameFromPath(std::wstring path)
{
    return path.substr(path.find_last_of(L"/\\") + 1);
}

void KillAll(HWND window, int actionID, int killReturnValue)
{
    if (IsWindow(window))
        KillAll(GetFullPath(window));
}

void KillAll(std::wstring process_name, int actionID, int killReturnValue)
{
    bool returnVal = false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (!hSnap)
        return;

    if (Process32First(hSnap, &pe))
    {
        bool bContinue = true;

        while (bContinue)
        {
            if (!wcscmp(pe.szExeFile, NameFromPath(process_name).c_str()) && !wcscmp(GetFullPath(pe.th32ProcessID).c_str(), process_name.c_str()))
            {
                if (!wcscmp(pe.szExeFile, L"explorer.exe") && IDOK == MessageBoxA(NULL, "do you really want to kill explorer.exe\nIt WILL break all!", "Jimmy critical message", MB_OKCANCEL | MB_TOPMOST))
                    Process_Tree(pe.th32ProcessID, actionID, killReturnValue);
            }

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }
    }
}

bool Process_Tree(DWORD pID, int actionID, int killReturnValue, HANDLE hSnap)
{
    bool returnVal = false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!hSnap && !(hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0))) //if (hsnap not exist) -> try to create it -> if (still not exist) -> then exit 
        return false;

    if (Process32First(hSnap, &pe))
    {
        bool bContinue = true;

        while (bContinue)
        {
            if (pe.th32ParentProcessID == pID)
                Process_Tree(pe.th32ProcessID, actionID, killReturnValue);//kill child proc tree

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }

        returnVal = Process(pID, actionID, killReturnValue);//kill parent
    }
    return returnVal;
}

bool Process(DWORD pID, int actionID, int killReturnValue)
{
    bool result;

    if (pvec == pID)
    {
        printf("ERROR:enable to kill process | REASON -> EL0-CPKT\n");
        return false;
    }

    if (actionID == PT_PAUSE)
    {
        result = DebugActiveProcess(pID) ? true : false;
        return result;
    }

    if (actionID == PT_RESUME)
    {
        result = DebugActiveProcessStop(pID) ? true : false;
        return result;
    }

    if (actionID == PT_KILL)
    {
        result = TerminateProcess(OpenProcess(PROCESS_TERMINATE, FALSE, pID), killReturnValue) ? true : false;
        return result;
    }

    return false;
}