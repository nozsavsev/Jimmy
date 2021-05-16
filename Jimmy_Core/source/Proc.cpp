#include "Jimmy_Core.h"

void Process_Only(HWND window, int ActionID, int killReturnValue)
{
    bool result = 0;
    DWORD pID = 0;
    if (IsWindow(window))
    {
        GetWindowThreadProcessId(window, &pID);
        result = Process_Tree(pID, ActionID, killReturnValue);
    }

    Log("processOnly -> %llu | Action:%d status: %s\n", window, ActionID, result ? "OK" : "ERROR");
}

void Process_All_Window(HWND window, int ActionID, int killReturnValue)
{
    if (IsWindow(window))
        Process_All(Name_From_Path(Get_Full_Path(window)).c_str(), false, ActionID, killReturnValue);
}


bool Process_All(std::wstring process_name, bool isPath, int ActionID, int killReturnValue)
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
                if (!_wcsicmp(Get_Full_Path(pe.th32ProcessID).c_str(), process_name.c_str()))
                    if (!Process_Tree(pe.th32ProcessID, ActionID, killReturnValue))
                        retVal = false;
            }
            else
                if (!_wcsicmp(pe.szExeFile, process_name.c_str()))
                    if (!Process_Tree(pe.th32ProcessID, ActionID, killReturnValue))
                        retVal = false;

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }
    }

    Log("Process_All -> %ws | Action:%d status: %s\n", process_name.c_str(), ActionID, retVal ? "OK" : "ERROR");
    return retVal;
}

bool Process_Tree(DWORD pID, int ActionID, int killReturnValue, HANDLE hSnap, VectorEx <DWORD>* pid_vec)
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
                Process_Tree(pe.th32ProcessID, ActionID, killReturnValue, hSnap, pid_vec);//kill child proc tree

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }

        if (!pid_vec->Contains(pID))
            pid_vec->push_back(pID);
    }

    if (first)
    {
        pid_vec->Foreach([&](DWORD pid_l)-> void { returnVal |= !Process(pid_l, ActionID, killReturnValue); });
        delete pid_vec;
    }

    return !returnVal;
}

bool Process(DWORD pID, int ActionID, int killReturnValue)
{
    bool result = false;

    if (pID == GetCurrentProcessId())
        return false;

    if (ActionID == PT_PAUSE)
    {
        result = DebugActiveProcess(pID);
        Log("Process -> pID:%d Path:'%ws' ActionID:%d status: %s\n", pID, Get_Full_Path(pID).c_str(), ActionID, result ? "OK" : "ERROR");
    }
    if (ActionID == PT_RESUME)
    {
        result = DebugActiveProcessStop(pID);
        Log("Process -> pID:%d Path:'%ws' ActionID:%d status: %s\n", pID, Get_Full_Path(pID).c_str(), ActionID, result ? "OK" : "ERROR");
    }

    if (ActionID == PT_KILL)
    {
        HANDLE phandle = OpenProcess(PROCESS_TERMINATE, 0, pID);

        result = TerminateProcess(phandle, killReturnValue);
        Log("Process -> pID:%d Path:'%ws' ActionID:%d status: %s | GLR:%lu\n", pID, Get_Full_Path(pID).c_str(), ActionID, result ? "OK" : "ERROR", GetLastError());

        if (phandle)
            CloseHandle(phandle);


    }

    return result;
}