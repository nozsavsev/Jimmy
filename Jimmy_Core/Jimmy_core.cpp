#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <windows.h>
#include <tlhelp32.h>

#define _CRT_SECURE_NO_WARNINGS

#include "hotkeyPP.h"

using namespace HKPP;
using namespace HKPP::extra;


#define PT_PAUSE  0
#define PT_RESUME 1
#define PT_KILL   2


bool Process(DWORD pID, int actionID, int killReturnValue);
bool Process_Tree(DWORD pID, int actionID = PT_KILL, int killReturnValue = -1, HANDLE hSnap = NULL);
void KillAll(std::wstring process_name, int actionID = PT_KILL, int killReturnValue = -1);

int main(int argc, char** argv)
{
    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();

    mng->HKPP_Init();

    mng->Add(HKPP::Hotkey_Deskriptor({ VK_LWIN , 'C' }, Hotkey_Settings_t(GetCurrentThreadId(), HKPP_BLOCK_INPUT, HKPP_ALLOW_INJECTED, WM_HKPP_DEFAULT_CALLBACK_MESSAGE, L"notepad.exe")));

    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);


        if (msg.message == WM_HKPP_DEFAULT_CALLBACK_MESSAGE)
        {
            if (msg.lParam)
            {
                Hotkey_Deskriptor* dsk = (Hotkey_Deskriptor*)msg.lParam;

                wprintf(L"kill \"%s\" %s\n", dsk->settings.name.c_str(), dsk->Real ? L"Allowed" : L"Denied");

                if (dsk->Real)
                {

                    KillAll(dsk->settings.name);


                }


                delete dsk;
            }
        }


        DispatchMessageW(&msg);
    }

    return 0;
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
            if (!wcscmp(pe.szExeFile, process_name.c_str()))
                Process_Tree(pe.th32ProcessID, actionID, killReturnValue);

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }
    }

}

bool Process(DWORD pID, int actionID, int killReturnValue)
{
    bool result;

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

bool Process_Tree(DWORD pID, int actionID, int killReturnValue, HANDLE hSnap)
{
    bool returnVal = false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!hSnap && !(hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)))
        return false;

    if (Process32First(hSnap, &pe))
    {
        bool bContinue = true;

        while (bContinue)
        {
            if (pe.th32ParentProcessID == pID)
                Process_Tree(pe.th32ProcessID, actionID, killReturnValue);

            bContinue = Process32Next(hSnap, &pe) ? true : false;
        }
        returnVal = Process(pID, actionID, killReturnValue);
    }
    return returnVal;
}