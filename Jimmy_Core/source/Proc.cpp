#include "Jimmy_Core.h"

void ProcessOnly(HWND window, int actionID, int killReturnValue)
{
    bool result = 0;
    DWORD pID = 0;
    if (IsWindow(window))
    {
        GetWindowThreadProcessId(window, &pID);
        result = ProcessTree(pID, actionID, killReturnValue);
    }

    log("processOnly -> %llu | Action:%d status: %s\n", window, actionID, result ? "OK" : "ERROR");
}

void ProcessAll_Window(HWND window, int actionID, int killReturnValue)
{
    if (IsWindow(window))
        ProcessAll(NameFromPath(GetFullPath(window)).c_str(), false, actionID, killReturnValue);
}
bool ProcessAll(std::wstring process_name, bool isPath, int actionID, int killReturnValue)
{
    bool retVal = true;

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
            if (isPath)
            {
                if (!_wcsicmp(GetFullPath(pe.th32ProcessID).c_str(), process_name.c_str()))
                    if (!ProcessTree(pe.th32ProcessID, actionID, killReturnValue))
                        retVal = false;
            }
            else
                if (!_wcsicmp(pe.szExeFile, process_name.c_str()))
                    if (!ProcessTree(pe.th32ProcessID, actionID, killReturnValue))
                        retVal = false;

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }
    }

    log("processALL -> %ws | Action:%d status: %s\n", process_name.c_str(), actionID, retVal ? "OK" : "ERROR");
    return retVal;
}

bool ProcessTree(DWORD pID, int actionID, int killReturnValue, HANDLE hSnap, VectorEx <DWORD>* pid_vec)
{
    bool first = false;
    bool returnVal = false;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (pid_vec == nullptr)
    {
        pid_vec = new VectorEx <DWORD>;
        first = true;
    }

    if (hSnap == nullptr && (hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == nullptr) //if (hsnap not exist) -> try to create it -> if (still not exist) -> then exit 
        return false;

    if (Process32First(hSnap, &pe))
    {
        bool bContinue = true;

        while (bContinue)
        {
            if (pe.th32ParentProcessID == pID)
                ProcessTree(pe.th32ProcessID, actionID, killReturnValue, hSnap, pid_vec);//kill child proc tree

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }

        if (!pid_vec->Contains(pID))
            pid_vec->push_back(pID);
    }

    if (first)
    {
        pid_vec->foreach([&](DWORD pid_l)-> void { returnVal |= !Process(pid_l, actionID, killReturnValue); });
        delete pid_vec;
    }

    return !returnVal;
}

bool Process(DWORD pID, int actionID, int killReturnValue)
{
    bool result = false;

    if (actionID == PT_PAUSE)
    {
        result = DebugActiveProcess(pID);
        log("Process -> pID:%d Path:'%ws' ActionID:%d status: %s\n", pID, GetFullPath(pID).c_str(), actionID, result ? "OK" : "ERROR");
    }
    if (actionID == PT_RESUME)
    {
        result = DebugActiveProcessStop(pID);
        log("Process -> pID:%d Path:'%ws' ActionID:%d status: %s\n", pID, GetFullPath(pID).c_str(), actionID, result ? "OK" : "ERROR");
    }

    if (actionID == PT_KILL)
    {
        HANDLE phandle = OpenProcess(PROCESS_TERMINATE, 0, pID);

        result = TerminateProcess(phandle, killReturnValue);
        log("Process -> pID:%d Path:'%ws' ActionID:%d status: %s | GLR:%lu\n", pID, GetFullPath(pID).c_str(), actionID, result ? "OK" : "ERROR" , GetLastError());

        if (phandle)
            CloseHandle(phandle);

    
    }

    return result;
}