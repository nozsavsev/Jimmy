#define _CRT_SECURE_NO_WARNINGS
#include "Jimmy_Core.h"
#include <iostream>

void helper_thread();
void deprecated_helper_thread();
void Exit_Jimmy();

JimmyGlobalProps_t Jimmy_Global_properties;
std::mutex Jimmy_Global_properties_mutex;

std::atomic<DWORD> mouse_proc_thread_id = 0;
std::atomic<DWORD> helper_thread_id = 0;

std::atomic<bool> Jimmy_Working = true;

void log(const char* log_str, ...)
{
    static std::mutex log_mutex;
    static FILE* flog = NULL;

    static char buffer[80];
    static time_t rawtime;
    static struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d/%m/%Y | %H:%M:%S", timeinfo);

    log_mutex.lock();

    if (flog == nullptr)
    {
#ifdef DEBUG
        flog = stderr;
#else
        fopen_s(&flog, "jimmy_log.txt", "a+");

        if (!flog)
            flog = stderr;

        fprintf(flog, "\n\n\n%s\tLOG START\n\n\n", buffer);
#endif // DEBUG
    }

    else if (log_str == nullptr)
        if (flog != stderr)
        {
            fprintf(flog, "\n\n\n%s\tLOG END --\n\n\n", buffer);
            fclose(flog);
            return;
        }


    fprintf(flog, "%s: ", buffer);
    va_list argptr;
    va_start(argptr, log_str);
    vfprintf(flog, log_str, argptr);
    va_end(argptr);

    log_mutex.unlock();
}



int main(int argc, char** argv)
{

    log("Starting Jimmy\n");
    Jimmy_Working = true;
    Sys_Init(argc, argv);

    helper_thread();

    //  deprecated_helper_thread();

    return 0;
}

void helper_thread()
{
    mouse_proc_thread_id.store(GetCurrentThreadId());
    HKPP::Hotkey_Manager* PInst = HKPP::Hotkey_Manager::Get_Instance();

    log("Setting up hooks (mouseLL)\n");
    size_t callback_uuid = PInst->Add_Callback([&](int i, WPARAM w, LPARAM l, VectorEx<key_deskriptor> keydesk, bool repeated_input) -> bool { return JimmyLowLevelKeyboardProc(i, w, l, keydesk, repeated_input); });
    log("MOUSE_LL callback added\n");

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
        Jimmy_Global_properties_mutex.lock();

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
        Jimmy_Global_properties_mutex.unlock();

        system("cls");
        HKPP::Hotkey_Manager::GetKeyboardState().foreach([&](key_deskriptor dsk) -> void { printf("[%ws]", keyToStr(dsk.Key).c_str()); });
        Sleep(100);
    }

    log("Helper thread exit\n");
    helper_thread_id.store(0);
}

LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{

    static JimmyGlobalProps_t local_props;

    Jimmy_Global_properties_mutex.lock();
    local_props = Jimmy_Global_properties;
    Jimmy_Global_properties_mutex.unlock();

    if (!local_props.BlockInjected_Mouse)
        return CallNextHookEx(NULL, nCode, wParam, lParam);

    if (nCode == HC_ACTION)
    {
        MSLLHOOKSTRUCT* kbd = (MSLLHOOKSTRUCT*)lParam;

        if ((((kbd->flags & LLMHF_INJECTED) == LLMHF_INJECTED) || ((kbd->flags & LLMHF_LOWER_IL_INJECTED) == LLMHF_LOWER_IL_INJECTED)))
            return 1;
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK JimmyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam, VectorEx<key_deskriptor> keydesk, bool repeated_input)
{
    static JimmyGlobalProps_t local_props;

    Jimmy_Global_properties_mutex.lock();
    local_props = Jimmy_Global_properties;
    Jimmy_Global_properties_mutex.unlock();

    if (!local_props.BlockInjected_Keyboard)
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