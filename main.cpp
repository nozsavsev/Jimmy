#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <thread>
#include "services.h"

#include <iostream>

std::mutex exit_state_mutex;
bool       exit_state;

void help();

int main(int argc, char* argv[])
{
    exit_state = false;

    Sys_Init(argc, argv);

    std::thread TePaR_Service_thread(&TPR_service);
    std::thread Window_Manage_thread(&Window_Manage_service);
    std::thread Media_Overlay_thread(&Media_Overlay_service);
    std::thread Locker_thread(&Locker);

    while (1)
    {
        if (isKeyPressed(GLB_ACTIVATE_KEY))
        {
            if (isKeyPressed(GLB_EXIT_KEY))
            {
                exit_state_mutex.lock();
                exit_state = true;
                exit_state_mutex.unlock();

                Sleep(1000);

                TePaR_Service_thread.detach();
                Window_Manage_thread.detach();
                Media_Overlay_thread.detach();
                Locker_thread.detach();

                exit(0);
            }

            if (isKeyPressed('J'))
                help();
        }

        Sleep(GLB_DEFAULT_TIMEOUT);
    }
    return 0;
}

void help()
{
    wchar_t* usage[1024];

    wsprintfW
    (
        (LPWSTR)usage,

        L"LALT + T - terminate process tree of active window\n"
        L"LALT + P - pause process     tree of active window\n"
        L"LALT + R - resume paused process stree\n"
        L"  *if you paused process tree you have to resume it berore pause others\n\n"
        L"LALT + H - hide active window\n"
        L"LALT + S - show hidden window\n"
        L"LALT + F - make active window topmost\n"
        L"LALT + F - on topmost window to activate agressive topmost\n"
        L"  *only one window can be agressive topmost\n"
        L"LALT + I - make active window topmost and transparent (ALPHA = 60)\n"
        L"LALT + B - make active window NO topmost and NO transparent\n\n"
        L"LALT + M - minimize active window\n"
        L"RCONTROL - to get fullscreen topmost window on all monitors\n"
        L" *useful if you need to hide what is happening on the monitor \n"
        L" *Press Q to close this full screen window \n"
        L"LALT + Q - exit\n"

    );

    MessageBoxW(NULL, (LPWSTR)usage, L"jimmy - help", MB_OK | MB_TOPMOST);
}
