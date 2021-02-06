#include <atlbase.h>
#include <atlconv.h>
#include <ShObjIdl.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>


#include "external/cJSON/cJSON.h"
#include "external/HotkeyPP/inc/hotkeyPP.h"

#include "keyboard_helpers.h"
#include "npath.h"
#include "proc.h"
#include "sys_init.h"

using namespace HKPP;
using namespace HKPP::extra;

std::atomic_bool Jimmy_Global_BlockInjected_Mouse = false;
std::atomic_bool Jimmy_Global_BlockInjected_Keyboard = false;

LRESULT CALLBACK JimmyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

void LoadConfig();
void standartCommandParcer(cJSON* command);

void hook_proc_th();
void LoadConfig();
void Media_Overlay_service();

int main(int argc, char** argv)
{
    Sys_Init(argc, argv);
    LoadConfig();

    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();
    std::thread th(&hook_proc_th);
    std::thread t2h(&Media_Overlay_service);

    mng->Add(HKPP::Hotkey_Deskriptor(
        { VK_LMENU , 'Q' },
        Hotkey_Settings_t(
            GetCurrentThreadId(),
            HKPP_BLOCK_INPUT,
            HKPP_ALLOW_INJECTED,
            WM_HKPP_DEFAULT_CALLBACK_MESSAGE,
            L"Exit jimmy",
            [&](Hotkey_Deskriptor d) -> void { exit(0); })
    ));


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

    mng->HKPP_Stop();
    return 0;
}

int GetHKPP_ConstantFromString(char* str)
{
    if (!strcmp(str, "HKPP_BLOCK_INPUT"))
        return HKPP_BLOCK_INPUT;
    if (!strcmp(str, "HKPP_ALLOW_INPUT"))
        return HKPP_ALLOW_INPUT;
    if (!strcmp(str, "HKPP_ALLOW_INJECTED"))
        return HKPP_ALLOW_INJECTED;
    if (!strcmp(str, "HKPP_DENY_INJECTED"))
        return HKPP_DENY_INJECTED;
}


std::wstring to_wstr(char const* input)
{
    return std::wstring(CA2W(input, CP_UTF8));
}

std::string to_str(wchar_t const* input)
{
    return std::string(CW2A(input, CP_UTF8));
}



void StandartHotkeyHandler(std::wstring command_str)
{
    cJSON* commands = cJSON_Parse(to_str(command_str.c_str()).c_str());

    for (int i = 0; i < cJSON_GetArraySize(commands); i++)
    {
        cJSON* subitem = cJSON_GetArrayItem(commands, i);

        if (subitem)
            standartCommandParcer(subitem);
    }

    if (commands)
        cJSON_Delete(commands);
}

void standartCommandParcer(cJSON* command)
{
    cJSON* action = cJSON_GetObjectItem(command, "Action");
    cJSON* status = cJSON_GetObjectItem(command, "Status");
    cJSON* subject = cJSON_GetObjectItem(command, "Subject");
    cJSON* type = cJSON_GetObjectItem(command, "Type");

    if (!action || !status || !subject || !type)
        return;

    if (!strcmp(action->valuestring, "KillAll") || !strcmp(action->valuestring, "Pause") || !strcmp(action->valuestring, "Resume"))
    {
        int actionID = 0;

        if (!strcmp(action->valuestring, "KillAll"))
            actionID = PT_KILL;
        else if (!strcmp(action->valuestring, "Pause"))
            actionID = PT_PAUSE;
        else if (!strcmp(action->valuestring, "Resume"))
            actionID = PT_RESUME;

        if (!strcmp(type->valuestring, "Concept"))
        {
            HWND window = NULL;

            if (!strcmp(subject->valuestring, "CurrentWindow"))
                window = GetForegroundWindow();
            else if (!strcmp(subject->valuestring, "UnderMouseWindow"))
            {
                POINT P;
                GetCursorPos(&P);
                window = WindowFromPoint(P);
            }

            KillAll(window, actionID);
        }

        else if (!strcmp(type->valuestring, "Path"))
            KillAll(to_wstr(subject->valuestring), true, actionID);

        else if (!strcmp(type->valuestring, "ExeName"))
            KillAll(to_wstr(subject->valuestring), false, actionID);
    }
    else if (!strcmp(action->valuestring, "Topmost") || !strcmp(action->valuestring, "Notopmost") || !strcmp(action->valuestring, "Minimize"))
    {
        HWND window = NULL;

        if (!strcmp(subject->valuestring, "CurrentWindow"))
            window = GetForegroundWindow();
        else if (!strcmp(subject->valuestring, "UnderMouseWindow"))
        {
            POINT P;
            GetCursorPos(&P);
            window = WindowFromPoint(P);
        }

        if (window)
        {
            if (!strcmp(action->valuestring, "Topmost"))
                SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

            else if (!strcmp(action->valuestring, "NoTopmost"))
                SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

            else if (!strcmp(action->valuestring, "Minimize"))
                ShowWindow(window, SW_FORCEMINIMIZE);
        }
    }

    else if (!strcmp(action->valuestring, "ToggleInputBlocking"))
    {
        if (!strcmp(subject->valuestring, "All"))
            Jimmy_Global_BlockInjected_Mouse = Jimmy_Global_BlockInjected_Keyboard = true;

        else if (!strcmp(subject->valuestring, "Mouse"))
            Jimmy_Global_BlockInjected_Mouse = true;

        else if (!strcmp(subject->valuestring, "Keyboard"))
            Jimmy_Global_BlockInjected_Keyboard = true;
    }
}

void LoadConfig()
{
    cJSON* config = NULL;
    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();

    {
        char* config_str;
        FILE* fp = NULL;
        size_t size = 0;

        fopen_s(&fp, "C:\\Users\\nozsavsev\\source\\repos\\Jimmy\\Jimmy_Core\\build\\bin\\Jimmy_config.json", "rb");

        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            config_str = (char*)malloc(size + 1);
            fread(config_str, 1, size, fp);
            config_str[size] = 0;

            config = cJSON_Parse(config_str);

            free(config_str);
            fclose(fp);
        }
    }

    if (config)
    {
        cJSON* combinations = cJSON_GetObjectItem(config, "combinations");

        if (!combinations)
            printf("broken config - combination array not found\n");


        for (int i = 0; i < cJSON_GetArraySize(combinations); i++)
        {
            cJSON* subitem = cJSON_GetArrayItem(combinations, i);

            if (!subitem)
            {
                printf("broken config - combination %d\n", i);
                continue;
            }
            cJSON* Name = cJSON_GetObjectItem(subitem, "Name");
            cJSON* BlockInputMode = cJSON_GetObjectItem(subitem, "BlockInputMode");
            cJSON* AllowInjectedI = cJSON_GetObjectItem(subitem, "AllowInjectedI");
            cJSON* Keys = cJSON_GetObjectItem(subitem, "Keys");
            cJSON* Actions = cJSON_GetObjectItem(subitem, "Actions");

            if (!BlockInputMode || !AllowInjectedI || !Keys || !Actions)
            {
                printf("broken config - combination %d\n", i);
                continue;
            }


            Hotkey_Settings_t settings;
            settings.Thread_Id = GetCurrentThreadId();
            settings.Msg = WM_HKPP_DEFAULT_CALLBACK_MESSAGE;

            char* strin = cJSON_Print(Actions);
            settings.name = to_wstr(strin);
            free(strin);

            settings.Block_Input = GetHKPP_ConstantFromString(BlockInputMode->valuestring);
            settings.Allow_Injected = GetHKPP_ConstantFromString(AllowInjectedI->valuestring);;
            settings.user_callback = [&](Hotkey_Deskriptor desk) -> void {StandartHotkeyHandler(desk.settings.name); };

            VectorEx <key_deskriptor> keys;

            printf("%60s ", Name->valuestring);
            for (int i = 0; i < cJSON_GetArraySize(Keys); i++)
            {
                cJSON* subitem = cJSON_GetArrayItem(Keys, i);
                if (subitem)
                {
                    DWORD key = StrToKey(to_wstr(subitem->valuestring));
                    if (key)
                        keys.push_back(key);

                    if (i)
                        printf(" +");
                    printf(" [%s]", subitem->valuestring);
                }
            }
            printf("\n");

            if (keys.size())
                mng->Add(Hotkey_Deskriptor(keys, settings));


        }

    }
    if (config)
        cJSON_Delete(config);
}

void hook_proc_th()
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

LRESULT CALLBACK JimmyLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!Jimmy_Global_BlockInjected_Mouse)
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
    if (!Jimmy_Global_BlockInjected_Keyboard)
        return false;

    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

        if ((((kbd->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kbd->flags & LLKHF_INJECTED) == LLKHF_INJECTED)))
            return true;
    }

    return false;
}






void Delete_Taskbar_Icon(HWND handle)
{
    ITaskbarList* pTaskList = NULL;
    HRESULT       createRet = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (LPVOID*)&pTaskList);

    CoInitialize(NULL);
    pTaskList->DeleteTab(handle);
    pTaskList->Release();
    CoUninitialize();
}

void Disable_Overlay()
{
    HWND hwndHost = NULL;

    while ((hwndHost = FindWindowExW(0, hwndHost, L"NativeHWNDHost", L"")) != NULL)
        if (FindWindowExW(hwndHost, NULL, L"DirectUIHWND", L"") != NULL)
            ShowWindow(hwndHost, SW_FORCEMINIMIZE);
}

void Enable_Overlay()
{
    HWND hwndHost = NULL;

    while ((hwndHost = FindWindowExW(0, hwndHost, L"NativeHWNDHost", L"")) != NULL)
        if (FindWindowExW(hwndHost, NULL, L"DirectUIHWND", L"") != NULL)
        {
            ShowWindow(hwndHost, SW_NORMAL);
            ShowWindow(hwndHost, SW_HIDE);
        }
}

float Get_Volume()
{
    float current_volume;
    IMMDeviceEnumerator* deviceEnumerator = NULL;
    IMMDevice* defaultDevice = NULL;
    IAudioEndpointVolume* endpointVolume = NULL;

    CoInitialize(NULL);

    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);

    deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    deviceEnumerator->Release();
    deviceEnumerator = NULL;

    defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
    defaultDevice->Release();
    defaultDevice = NULL;


    endpointVolume->GetMasterVolumeLevelScalar(&current_volume);
    current_volume *= 100;

    endpointVolume->Release();
    endpointVolume = NULL;

    CoUninitialize();

    return current_volume;
}

bool Set_Volume(float new_bolume)
{
    new_bolume /= 100;

    IMMDeviceEnumerator* deviceEnumerator = NULL;
    IMMDevice* defaultDevice = NULL;
    IAudioEndpointVolume* endpointVolume = NULL;

    CoInitialize(NULL);

    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);

    deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    deviceEnumerator->Release();
    deviceEnumerator = NULL;

    defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
    defaultDevice->Release();
    defaultDevice = NULL;

    endpointVolume->SetMasterVolumeLevelScalar(new_bolume, NULL);

    float result_volume;
    endpointVolume->GetMasterVolumeLevelScalar(&result_volume);

    endpointVolume->Release();
    endpointVolume = NULL;

    CoUninitialize();

    if (result_volume == new_bolume) return true;
    else                             return false;
}

void Media_Overlay_service()
{
    float old_volume = Get_Volume();
    Disable_Overlay();
    while (1)
    {
        old_volume = Get_Volume();

        while (old_volume == Get_Volume())
        {
            Sleep(20);
        }
        Disable_Overlay();
    }
}