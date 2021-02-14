#define _CRT_SECURE_NO_WARNINGS
#include "Jimmy_Core.h"

void MouseHook_proc_helper_thread();
void helper_thread();

std::atomic<JimmyGlobalProps_t> Jimmy_Global_properties;

std::atomic<DWORD> th_id = 0;

void log(const char* log_str, ...)
{
    static std::mutex log_mutex;
    static FILE* flog = NULL;

    static char buffer[80];
    static time_t rawtime;
    static struct tm* timeinfo;

    log_mutex.lock();

    if (!flog)
    {
        fopen_s(&flog, "jimmy_log.txt", "wt");

        if (!flog)
            flog = stdout;
    }


    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);

    fprintf(flog, "%s:", buffer);
    va_list argptr;
    va_start(argptr, log_str);
    vfprintf(flog, log_str, argptr);
    va_end(argptr);

    log_mutex.unlock();
}


int main(int argc, char** argv)
{
    log("starting Jimmy\n");

    Sys_Init(argc, argv);
    
    if (LoadConfig(0))
        log("confug loaded\n");


    std::thread hook_helper_thread(&MouseHook_proc_helper_thread);
    auto mng = HKPP::Hotkey_Manager::Get_Instance();
    mng->Add_Hotkey(HKPP::Hotkey_Deskriptor({ VK_LMENU , 'Q' }, Hotkey_Settings_t(
        L"Exit jimmy",
        [&](Hotkey_Deskriptor d) -> void {Enable_Overlay(); exit(0); },
        NULL,
        HKPP_BLOCK_INPUT,
        HKPP_DENY_INJECTED)));

    helper_thread();

    hook_helper_thread.join();
    return 0;
}

void MouseHook_proc_helper_thread()
{

    th_id.store(GetCurrentThreadId());

    HKPP::Hotkey_Manager* PInst = HKPP::Hotkey_Manager::Get_Instance();


    log("setting up mouse hook\n");
    size_t callback_uuid = PInst->Add_Callback([&](int i, WPARAM w, LPARAM l) -> bool { return JimmyLowLevelKeyboardProc(i, w, l); });
    HHOOK mouse_hook_handle = SetWindowsHookExW(WH_MOUSE_LL, JimmyLowLevelMouseProc, NULL, NULL);
    log("mouse hook OK\n");

    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        if (msg.message == WM_QUIT) break;
        DispatchMessageW(&msg);
    }

    PInst->Remove_Callback(callback_uuid);
    callback_uuid = 0;
    UnhookWindowsHookEx(mouse_hook_handle);
}

void helper_thread()
{
    log("Helper started\n");

    Enable_Overlay();

    float volNowOld = Get_Volume();
    bool IsHidden = true;

    while (1)
    {
        if (Jimmy_Global_properties.load().MediaOverlayServiceTrue)
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
















void WatchDirectory(LPTSTR lpDir)
{
    DWORD dwWaitStatus;
    HANDLE dwChangeHandles[2];
    TCHAR lpDrive[4];
    TCHAR lpFile[_MAX_FNAME];
    TCHAR lpExt[_MAX_EXT];

    _tsplitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

    lpDrive[2] = (TCHAR)'\\';
    lpDrive[3] = (TCHAR)'\0';

    // Watch the directory for file creation and deletion. 

    dwChangeHandles[0] = FindFirstChangeNotification(
        lpDir,                         // directory to watch 
        FALSE,                         // do not watch subtree 
        FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 

    if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
    {
        printf("\n ERROR: FindFirstChangeNotification function failed.\n");
        ExitProcess(GetLastError());
    }


    dwChangeHandles[1] = FindFirstChangeNotification(L"C:\\Users\\nozsavsev\\Desktop", FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

    //    switch (dwWaitStatus)
    //    {
    //    case WAIT_OBJECT_0:
    //        printf("changed");
    //        break;
    //
    //    case WAIT_TIMEOUT:
    //        break;
    //
    //    default:
    //        break;
    //    }
}
