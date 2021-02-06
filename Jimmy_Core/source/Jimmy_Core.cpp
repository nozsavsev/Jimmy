#include "Jimmy_Core.h"

void MouseHook_proc_helper_thread(); //local 
void helper_thread();                //local 
std::atomic<JimmyGlobalProps_t> Jimmy_Global_properties;

int main(int argc, char** argv)
{
    Sys_Init(argc, argv);
    LoadConfig();
    //default exit combination
    auto mng = HKPP::Hotkey_Manager::Get_Instance();
    mng->Add(HKPP::Hotkey_Deskriptor({ VK_LMENU , 'Q' }, Hotkey_Settings_t(GetCurrentThreadId(), HKPP_BLOCK_INPUT, HKPP_ALLOW_INJECTED, WM_HKPP_DEFAULT_CALLBACK_MESSAGE, L"Exit jimmy", [&](Hotkey_Deskriptor d) -> void {Enable_Overlay(); exit(0); })));

    std::thread hook_helper_thread(&MouseHook_proc_helper_thread);
    std::thread helper_thread(&helper_thread);

    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);

        if (msg.message == WM_HKPP_DEFAULT_CALLBACK_MESSAGE)
        {
            if (msg.lParam)
            {
                Hotkey_Deskriptor* dsk = (Hotkey_Deskriptor*)msg.lParam;
                dsk->settings.user_callback(*dsk);
                delete dsk;
            }
        }

        DispatchMessageW(&msg);
    }

    return 0;
}



void MouseHook_proc_helper_thread()
{
    HKPP::Hotkey_Manager::Get_Instance()->Add_Callback([&](int i, WPARAM w, LPARAM l) -> bool { return JimmyLowLevelKeyboardProc(i, w, l); }, 2258);
    HHOOK mouse_hook_handle = SetWindowsHookExW(WH_MOUSE_LL, JimmyLowLevelMouseProc, NULL, NULL);

    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(mouse_hook_handle);
}

void helper_thread()
{
    Enable_Overlay();

    float volNowOld = Get_Volume();
    bool IsHidden = true;

    while (1)
    {
        if (Jimmy_Global_properties.load().MediaOverlayServiceEnabled)
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
}

LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!Jimmy_Global_properties.load().BlockInjected_Mouse)
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
    if (!Jimmy_Global_properties.load().BlockInjected_Keyboard)
        return false;

    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

        if ((((kbd->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kbd->flags & LLKHF_INJECTED) == LLKHF_INJECTED)))
            return true;
    }

    return false;
}