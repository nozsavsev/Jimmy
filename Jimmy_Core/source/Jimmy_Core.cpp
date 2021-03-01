#define _CRT_SECURE_NO_WARNINGS
#include "Jimmy_Core.h"

void MouseHook_proc_helper_thread();
void helper_thread();
void Exit_Jimmy();

std::atomic<JimmyGlobalProps_t> Jimmy_Global_properties;

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

    log_mutex.lock();

    if (flog == nullptr)
    {
#ifdef DEBUG
        flog = stderr;
#else
        fopen_s(&flog, "jimmy_log.txt", "wt+");

        if (!flog)
            flog = stderr;
#endif // DEBUG
    }

    else if (log_str == nullptr)
        if (flog != stderr)
        {
            fclose(flog);
            return;
        }

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d/%m/%Y | %H:%M:%S", timeinfo);

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

    if (LoadConfig(0))
        log("Config loaded\n");

    //std::thread hook_helper_thread(&MouseHook_proc_helper_thread);

    auto mng = HKPP::Hotkey_Manager::Get_Instance();
    mng->Add_Hotkey(HKPP::Hotkey_Deskriptor({ VK_LMENU , 'Q' }, Hotkey_Settings_t(
        L"Exit jimmy",
        [&](Hotkey_Deskriptor d) -> void {Exit_Jimmy(); },
        NULL,
        HKPP_BLOCK_INPUT,
        HKPP_DENY_INJECTED)));


    helper_thread();

    return 0;
}

void MouseHook_proc_helper_thread()
{
    mouse_proc_thread_id.store(GetCurrentThreadId());
    HKPP::Hotkey_Manager* PInst = HKPP::Hotkey_Manager::Get_Instance();

    log("Setting up hooks (mouseLL)\n");
    size_t callback_uuid = PInst->Add_Callback([&](int i, WPARAM w, LPARAM l) -> bool { return JimmyLowLevelKeyboardProc(i, w, l); });
    HHOOK mouse_hook_handle = SetWindowsHookExW(WH_MOUSE_LL, JimmyLowLevelMouseProc, NULL, NULL);
    log("Mouse hook OK\n");

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

void helper_thread()
{

    helper_thread_id.store(GetCurrentThreadId());
    log("Helper thread started\n");

    Enable_Overlay();

    float volNowOld = Get_Volume();
    bool IsHidden = false;

    while (Jimmy_Working)
    {
        if (Jimmy_Global_properties.load().MediaOverlayService)
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

        system("cls");
        HKPP::Hotkey_Manager::GetKeyboardState().foreach([&](key_deskriptor dsk) -> void { printf("[%ws]", keyToStr(dsk.Key).c_str()); });
        Sleep(10);
    }

    log("Helper thread exit\n");
    helper_thread_id.store(0);
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

void Exit_Jimmy()
{
    Jimmy_Working = false;
    Enable_Overlay();
    Hotkey_Manager::Get_Instance()->HKPP_Stop();

    int ExitTimeout = 1000;

    if (GetCurrentThreadId() != mouse_proc_thread_id.load())
    {
        PostThreadMessageW(mouse_proc_thread_id.load(), WM_QUIT, NULL, NULL);

        while (!mouse_proc_thread_id.load())
        {
            static int elapsed = 0;
            elapsed += 10;
            Sleep(1);

            if (elapsed >= ExitTimeout)
            {
                break;
            }
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

void WatchDirectory(LPTSTR lpDir)
{
    DWORD WaitStatus = 0;;
    HANDLE ChangeHandle = FindFirstChangeNotificationW(L"C:\\Users\\nozsavsev\\Desktop\\Jimmy_config.json", FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

    WaitForSingleObject(ChangeHandle, 0);

    switch (WaitStatus)
    {
    case WAIT_OBJECT_0:
        printf("changed");
        break;
    }
}
