#include "proc.h"
#include "npath.h"

void KillAll(HWND window, int actionID, int killReturnValue)
{
    if (IsWindow(window))
        KillAll(GetFullPath(window), actionID, killReturnValue);
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
            if (!wcscmp(pe.szExeFile, NameFromPath(process_name).c_str()) && !wcscmp(GetFullPath(pe.th32ProcessID).c_str(), process_name.c_str()))//! TODO process wawre list
            {
                //if (!wcscmp(pe.szExeFile, L"explorer.exe") && IDOK == MessageBoxA(NULL, "do you really want to KILL \\ PAUSE \\ RESUME explorer.exe\nIt WILL break all!", "Jimmy critical message", MB_OKCANCEL | MB_TOPMOST))
                //    Process_Tree(pe.th32ProcessID, actionID, killReturnValue);
                //else
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
    bool result = false;

    if (actionID == PT_PAUSE)
        result = DebugActiveProcess(pID) ? true : false;

    if (actionID == PT_RESUME)
        result = DebugActiveProcessStop(pID) ? true : false;

    if (actionID == PT_KILL)
        result = TerminateProcess(OpenProcess(PROCESS_TERMINATE, FALSE, pID), killReturnValue) ? true : false;

    return result;
}