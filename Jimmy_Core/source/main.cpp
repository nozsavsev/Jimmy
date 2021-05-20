/*Copyright 2020 Nozdrachev Ilia
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Jimmy_Core.h"
#include <iostream>

void helper_thread();
void gui_loop();
void Exit_Jimmy();

JimmyGlobalProps_t Jimmy_Global_properties;

std::atomic<DWORD> mouse_proc_thread_id = 0;
std::atomic<DWORD> helper_thread_id = 0;

std::atomic<bool> Jimmy_Working = true;


int main(int argc, char** argv)
{
    Log("Starting Jimmy\n");
    Jimmy_Working = true;
    Sys_Init(argc, argv);

    HKPP::Hotkey_Manager::Get_Instance()->HKPP_Init();
    Log("HKPP inited\n");

    HKPP::Hotkey_Manager::Get_Instance()->Add_Hotkey(HKPP::Hotkey_Deskriptor({ VK_LMENU , 'Q' }, Hotkey_Settings_t(
        L"Exit jimmy",
        [&](Hotkey_Deskriptor d) -> void {Exit_Jimmy(); },
        NULL,
        HKPP_BLOCK_INPUT,
        HKPP_DENY_INJECTED)));
    Log("exit comb added\n");

    Jimmy_Global_properties.KeyboardLL_calback_uuid = HKPP::Hotkey_Manager::Get_Instance()->Add_Callback(
        [&](int i, WPARAM w, LPARAM l, VectorEx<key_deskriptor> keydesk, bool repeated_input) ->

        bool {
            return JimmyLowLevelKeyboardProc(i, w, l, keydesk, repeated_input);
        }
    );
    Log("Jimmy KeyboadHandler Added");


    if (Load_Config())
        Log("Config loaded\n");

    //std::thread th(&helper_thread);
    gui_loop();
    //th.join();

    return 0;
}

void helper_thread()
{
    mouse_proc_thread_id.store(GetCurrentThreadId());
    HKPP::Hotkey_Manager* PInst = HKPP::Hotkey_Manager::Get_Instance();

    Log("Setting up hooks mouseLL\n");
    HHOOK mouse_hook_handle = SetWindowsHookExW(WH_MOUSE_LL, JimmyLowLevelMouseProc, NULL, NULL);

    if (mouse_hook_handle)
    {
        Log("MouseLL hook OK\n");






        MSG msg;
        while (GetMessageW(&msg, NULL, NULL, NULL))
        {
            TranslateMessage(&msg);
            if (msg.message == WM_QUIT) break;

            DispatchMessageW(&msg);
        }
    }

    UnhookWindowsHookEx(mouse_hook_handle);
    Log("Disabling hooks (mouseLL)\n");
    Log("MouseLL exit\n");
    mouse_proc_thread_id.store(0);
}




void gui_loop()
{
    helper_thread_id.store(GetCurrentThreadId());
    Log("Helper thread started\n");

    Enable_Overlay();

    float volNowOld = Get_Volume();
    bool IsHidden = false;

    while (Jimmy_Working)
    {

        Locker_immproc();

        if (Jimmy_Global_properties.Media_Overlay_Service)
        {
            if (volNowOld != Get_Volume())
            {
                volNowOld = Get_Volume();
                Disable_Overlay();
                IsHidden = true;
            }
        }
        else if (IsHidden)
        {
            IsHidden = false;
            Enable_Overlay();
        }
    }

    Log("Helper thread exit\n");
    helper_thread_id.store(0);
}

LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!Jimmy_Global_properties.Block_Injected_Mouse)
        return CallNextHookEx(NULL, NULL, NULL, NULL);

    if (nCode == HC_ACTION)
    {
        MSLLHOOKSTRUCT* kbd = (MSLLHOOKSTRUCT*)lParam;

        if ((((kbd->flags & LLMHF_INJECTED) == LLMHF_INJECTED) || ((kbd->flags & LLMHF_LOWER_IL_INJECTED) == LLMHF_LOWER_IL_INJECTED)))
            return 1;
    }

    return CallNextHookEx(NULL, NULL, NULL, NULL);
}

LRESULT CALLBACK JimmyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam, VectorEx<key_deskriptor> keydesk, bool repeated_input)
{
    if (!Jimmy_Global_properties.Block_Injected_Keyboard)
        return false;

    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

        if ((((kbd->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kbd->flags & LLKHF_INJECTED) == LLKHF_INJECTED)))
            return true;
    }

    return false;
}



void Exit_Jimmy()
{
    Jimmy_Working = false;
    Enable_Overlay();
    Hotkey_Manager::Get_Instance()->HKPP_Stop();//will removecallback too

    const int ExitTimeout = 1000;

    if (GetCurrentThreadId() != mouse_proc_thread_id.load())
    {
        PostThreadMessageW(mouse_proc_thread_id.load(), WM_QUIT, NULL, NULL);

        while (!mouse_proc_thread_id.load())
        {
            static int elapsed = 0;
            elapsed += 10;
            Sleep(1);

            if (elapsed >= ExitTimeout)
                break;

        }
    }

    if (GetCurrentThreadId() != helper_thread_id.load())
    {
        PostThreadMessageW(helper_thread_id.load(), WM_QUIT, NULL, NULL);

        while (!helper_thread_id.load())
        {
            static int elapsed = 0;
            elapsed += 10;
            Sleep(10);

            if (elapsed >= ExitTimeout)
                break;
        }
    }
    Log("Exit\n");
    Log(nullptr);
    exit(0);
}