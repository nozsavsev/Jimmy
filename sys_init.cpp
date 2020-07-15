#include <SFML/Graphics.hpp>
#include "services.h"

void Sys_Init(int argc, char** argv)
{

    if (!Is_Running_As_Admin())
    {
        if (IDYES == MessageBoxW(NULL, L"you started jimmy without administrator privileges\npress Ok to restart winth admin rights", L"jimmy - accessibility alert", MB_YESNO | MB_TOPMOST))
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

    else
        if (argc > 1 && !strcmp(argv[1], "-restarted_by_enother_instance"))
        {
            MessageBoxW(NULL, L"admin access granted", L"jimmy - accessibility alert", MB_OK | MB_TOPMOST);
        }

    CreateMutex(NULL, true, L"JYMMY_GLOBAL_START_MUTEX");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxW(NULL, L"jimmy is alaredy running!", L"jimmy alert", MB_OK | MB_TOPMOST);
        exit(0);
    }
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

    return is_run_as_admin;
}