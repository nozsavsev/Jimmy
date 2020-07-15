#ifndef SERVICES_H
#define SERVICES_H

#include <windows.h>
#include <thread>
#include <mutex>
#include <vector>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#include "proc.h"

#define GLB_ACTIVATE_KEY  VK_LMENU
#define GLB_NON_RECOVERABLE_SECONDARY_ACTIVATE_KEY  VK_LCONTROL
#define GLB_DEFAULT_TIMEOUT 50

#define GLB_EXIT_KEY      'Q'

extern std::mutex exit_state_mutex;
extern bool       exit_state;


/* TPR service*/
#define TPR_TERMINATE_KEY 'T'
#define TPR_PAUSE_KEY     'L'
#define TPR_RESUME_KEY    'U'

void TPR_service();

/*window manage service*/
#define WMS_HIDE_KEY              'H'
#define WMS_SHOW_KEY              'S'

#define WMS_MINIMIZE_KEY          'M'
#define WMS_ACTIVATE_KEY          'A'

#define WMS_TOPMOST_KEY           'F'
#define WMS_INFO_TP_KEY           'I'
#define WMS_DIS_TOPMOST_KEY       'B'

void Window_Manage_service();

void Set_Transparency(HWND hwnd, char Transperancy);

/*volume overlay*/
void help_volume_service();
void Media_Overlay_service();

void Delete_Taskbar_Icon(HWND handle);

void Disable_Overlay();

float Get_Volume();
bool  Set_Volume(float nVolume);

extern std::mutex agersive_topmost_window_handle_mutex;
extern HWND       agersive_topmost_window_handle;

extern std::mutex         jimmy_hwnd_protected_mutex;
extern std::vector <HWND> jimmy_hwnd_protected;

/*locker*/
void Locker();

/*keyboard*/
bool isKeyPressed(int key);
void Wait_For_Key_Release(int key);
void Wait_For_Key_Down(int key);

/*sys init*/
bool Is_Running_As_Admin();

void Sys_Init(int argc, char** argv);

#endif//SERVICES_H