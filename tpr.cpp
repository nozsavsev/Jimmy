#include "services.h"

void TPR_service()
{
    pid_protected_mutex.lock();
    pid_protected.push_back(0);
    pid_protected[pid_protected.size() - 1] = GetCurrentProcessId();
    pid_protected_mutex.unlock();

    while (1)
    {
        Wait_For_Key_Down(GLB_ACTIVATE_KEY); //sleep(1); need to decrease CPU usage

        if (isKeyPressed(GLB_ACTIVATE_KEY))
        {
            if (isKeyPressed(TPR_TERMINATE_KEY))
            {
                DWORD pID;

                GetWindowThreadProcessId(GetForegroundWindow(), &pID);

                Process_Tree(pID, PT_PAUSE);
                Process_Tree(pID, PT_KILL);
                Process_Tree(pID, PT_RESUME);

                Wait_For_Key_Release(TPR_TERMINATE_KEY);
            }

            if (isKeyPressed(TPR_PAUSE_KEY))
            {
                DWORD pID;

                HWND window_selected = GetForegroundWindow();

                GetWindowThreadProcessId(window_selected, &pID);

                Process_Tree(pID, PT_PAUSE);

                Wait_For_Key_Release(TPR_PAUSE_KEY);

                while (window_selected)
                {
                    if (isKeyPressed(GLB_ACTIVATE_KEY))
                    {
                        exit_state_mutex.lock();
                        if (isKeyPressed(TPR_RESUME_KEY) || exit_state)
                        {
                            Process_Tree(pID, PT_RESUME);

                            Wait_For_Key_Release(TPR_RESUME_KEY);

                            window_selected = NULL;
                        }
                        exit_state_mutex.unlock();

                        if (isKeyPressed(TPR_TERMINATE_KEY))
                        {
                            Process_Tree(pID, PT_KILL);
                            Process_Tree(pID, PT_RESUME);

                            Wait_For_Key_Release(TPR_TERMINATE_KEY);

                            window_selected = NULL;
                        }
                    }
                }
            }
        }
    }
}