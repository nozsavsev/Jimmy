#define _CRT_SECURE_NO_WARNINGS
#include "Jimmy_Core.h"
#include <iostream>

void helper_thread();
void deprecated_helper_thread();
void Exit_Jimmy();

JimmyGlobalProps_t Jimmy_Global_properties;

std::atomic<DWORD> mouse_proc_thread_id = 0;
std::atomic<DWORD> helper_thread_id = 0;

std::atomic<bool> Jimmy_Working = true;


int main(int argc, char** argv)
{

    log("Starting Jimmy\n");
    Jimmy_Working = true;
    Sys_Init(argc, argv);

    std::thread th(&helper_thread);

    deprecated_helper_thread();

    th.join();

    return 0;
}

void helper_thread()
{
    mouse_proc_thread_id.store(GetCurrentThreadId());
    HKPP::Hotkey_Manager* PInst = HKPP::Hotkey_Manager::Get_Instance();

    log("Setting up hooks (mouseLL)\n");
    size_t callback_uuid = PInst->Add_Callback([&](int i, WPARAM w, LPARAM l, VectorEx<key_deskriptor> keydesk, bool repeated_input) -> bool { return JimmyLowLevelKeyboardProc(i, w, l, keydesk, repeated_input); });
    HHOOK mouse_hook_handle = SetWindowsHookExW(WH_MOUSE_LL, JimmyLowLevelMouseProc, NULL, NULL);
    log("Mouse hook OK\n");

    if (LoadConfig())
        log("Config loaded\n");

    HKPP::Hotkey_Manager::Get_Instance()->Add_Hotkey(HKPP::Hotkey_Deskriptor({ VK_LMENU , 'Q' }, Hotkey_Settings_t(
        L"Exit jimmy",
        [&](Hotkey_Deskriptor d) -> void {Exit_Jimmy(); },
        NULL,
        HKPP_BLOCK_INPUT,
        HKPP_DENY_INJECTED)));

    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        if (msg.message == WM_QUIT) break;

        if (msg.message == WM_HKPP_DEFAULT_CALLBACK_MESSAGE)
        {
            printf("acttttion\n");
            //do work

        }

        DispatchMessageW(&msg);
    }

    PInst->Remove_Callback(callback_uuid);
    log("Removing callbacks\n");
    callback_uuid = 0;
    UnhookWindowsHookEx(mouse_hook_handle);
    log("Disabling hooks (mouseLL)\n");
    log("MouseLL exit\n");
    mouse_proc_thread_id.store(0);
}




void deprecated_helper_thread()
{
    helper_thread_id.store(GetCurrentThreadId());
    log("Helper thread started\n");

    Enable_Overlay();

    float volNowOld = Get_Volume();
    bool IsHidden = false;

    while (Jimmy_Working)
    {

        if (Jimmy_Global_properties.MediaOverlayService)
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

        Sleep(10);
    }

    log("Helper thread exit\n");
    helper_thread_id.store(0);
}

LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!Jimmy_Global_properties.BlockInjected_Mouse)
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
    if (!Jimmy_Global_properties.BlockInjected_Keyboard)
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
    Hotkey_Manager::Get_Instance()->HKPP_Stop();

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
    log("Exit\n");
    log(nullptr);
    exit(0);
}