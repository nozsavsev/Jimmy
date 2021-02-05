#include "hotkeyPP.h"
#include "npath.h"
#include "proc.h"

using namespace HKPP;
using namespace HKPP::extra;

std::atomic_bool Jimmy_Global_BlockInjected = false;

LRESULT CALLBACK JimmyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

bool Is_Running_As_Admin();
void Sys_Init(int argc, char** argv);
void TPR_Init(DWORD pID);

void hook_proc_th()
{
    HHOOK mouse_hook_handle = SetWindowsHookExW(WH_MOUSE_LL, JimmyLowLevelMouseProc, NULL, NULL);
   
    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(mouse_hook_handle);
}

int main(int argc, char** argv)
{
    Sys_Init(argc, argv);

    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();

    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU , 'Q' },
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_ALLOW_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"Exit jimmy",
            [&](void) -> void { exit(0); })
    ));

    TPR_Init(GetCurrentThreadId());

    mng->Add_Callback([&](int i, WPARAM w, LPARAM l) -> bool { return JimmyLowLevelKeyboardProc(i, w, l); }, 2258);
    HHOOK mouse_hook_handle = SetWindowsHookExW(WH_MOUSE_LL, JimmyLowLevelMouseProc, NULL, NULL);

    std::thread th(&hook_proc_th);

    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU , 'B'},
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_DENY_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"Toggle injected input blocking",
            [&](void) -> void
            {
                bool local = Jimmy_Global_BlockInjected.load();
                local = local ? false : true;
                Jimmy_Global_BlockInjected.store(local);
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
                dsk->settings.user_callback();
                delete dsk;
            }
        }

        DispatchMessageW(&msg);
    }

    mng->HKPP_Stop();
    UnhookWindowsHookEx(mouse_hook_handle);

    return 0;
}

void TPR_Init(DWORD pID)
{

    auto mng = HKPP::Hotkey_Manager::Get_Instance();
    //PKILL
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


    //PAUSE RESUME
    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU , 'L' },
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_ALLOW_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"Freeze All Instances of current window",
            [&](void) -> void { KillAll(GetForegroundWindow(), PT_PAUSE); })
    ));

    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU ,VK_LSHIFT, 'L' },
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_ALLOW_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"Freeze All Instances of undermose window",
            [&](void) -> void
            {
                POINT P;
                GetCursorPos(&P);
                KillAll(WindowFromPoint(P), PT_PAUSE);
            })
    ));

    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU , 'U' },
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_ALLOW_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"UnFreeze All Instances of undermose window",
            [&](void) -> void
            {
                POINT P;
                GetCursorPos(&P);
                KillAll(WindowFromPoint(P), PT_RESUME);
            })
    ));
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

LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!Jimmy_Global_BlockInjected)
        return CallNextHookEx(NULL, NULL, NULL, NULL);

    if (nCode == HC_ACTION)
    {
        MSLLHOOKSTRUCT* kbd = (MSLLHOOKSTRUCT*)lParam;

        if ((((kbd->flags & LLMHF_INJECTED) == LLMHF_INJECTED) || ((kbd->flags & LLMHF_LOWER_IL_INJECTED) == LLMHF_LOWER_IL_INJECTED)))
            return 1;
    }

    return CallNextHookEx(NULL, NULL, NULL, NULL);
}

LRESULT CALLBACK JimmyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!Jimmy_Global_BlockInjected)
        return false;

    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

        if ((((kbd->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kbd->flags & LLKHF_INJECTED) == LLKHF_INJECTED)))
            return true;
    }

    return false;
}