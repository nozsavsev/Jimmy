#include "proc.h"

std::mutex pid_protected_mutex;

std::vector <DWORD> pid_protected;



bool isPidProtected(DWORD pID)
{
    for (int i = 0; i < pid_protected.size(); i++)
        if (pID == pid_protected[i])
            return true;

    return false;
}

bool Process(DWORD pID, int actionID, int killReturnValue)
{
    bool result;

    if (!isPidProtected(pID))
    {
        if (actionID == PT_PAUSE)
        {
            result = DebugActiveProcess(pID);
            return result;
        }

        if (actionID == PT_RESUME)
        {
            result = DebugActiveProcessStop(pID);
            return result;
        }

        if (actionID == PT_KILL)
        {
            result = TerminateProcess(OpenProcess(PROCESS_TERMINATE, FALSE, pID), killReturnValue);
            return result;
        }
    }
}

bool Process_Tree(DWORD pID, int actionID, int killReturnValue)
{
    bool returnVal = false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (!hSnap)
        return false;

    if (Process32First(hSnap, &pe))
    {
        bool bContinue = true;

        while (bContinue)
        {
            if (pe.th32ParentProcessID == pID)
                Process_Tree(pe.th32ProcessID, actionID, killReturnValue);

            bContinue = Process32Next(hSnap, &pe);
        }
        returnVal = Process(pID, actionID, killReturnValue);
    }
    return returnVal;
}