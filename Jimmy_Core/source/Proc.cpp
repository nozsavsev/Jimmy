#include "Jimmy_Core.h"

void ProcessOnly(HWND window, int actionID, int killReturnValue)
{
    DWORD pID = 0;
    if (IsWindow(window))
    {
        GetWindowThreadProcessId(window, &pID);
        ProcessTree(pID, actionID, killReturnValue);
    }

    printf("\n\n");
}

void ProcessAll(HWND window, int actionID, int killReturnValue)
{
    if (IsWindow(window))
        ProcessAll(GetFullPath(window), true, actionID, killReturnValue);
}

void ProcessAll(std::wstring process_name, bool isPath, int actionID, int killReturnValue)
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
            if (!wcscmp(pe.szExeFile, NameFromPath(process_name).c_str()))
            {
                if (wcscmp(pe.szExeFile, L"explorer.exe"))
                {
                    if (isPath && !wcscmp(GetFullPath(pe.th32ProcessID).c_str(), process_name.c_str()))
                        ProcessTree(pe.th32ProcessID, actionID, killReturnValue);
                    else if (!wcscmp(NameFromPath(GetFullPath(pe.th32ProcessID)).c_str(), process_name.c_str()))
                        ProcessTree(pe.th32ProcessID, actionID, killReturnValue);
                }
            }

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }
    }

    printf("\n\n");
}

bool ProcessTree(DWORD pID, int actionID, int killReturnValue, HANDLE hSnap)
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
                ProcessTree(pe.th32ProcessID, actionID, killReturnValue);//kill child proc tree

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }

        returnVal = Process(pID, actionID, killReturnValue);//kill parent
    }
    return returnVal;
}

bool Process(DWORD pID, int actionID, int killReturnValue)
{
    bool result = false;

    if (actionID == PT_PAUSE)
        result = DebugActiveProcess(pID);

    if (actionID == PT_RESUME)
        result = DebugActiveProcessStop(pID);

    if (actionID == PT_KILL)
    {
        HANDLE phandle = OpenProcess(PROCESS_TERMINATE, FALSE, pID);
        if (TerminateProcess(phandle, killReturnValue) != TRUE)
            printf("%d error %d\n", pID, GetLastError());
    }

    return result;
}