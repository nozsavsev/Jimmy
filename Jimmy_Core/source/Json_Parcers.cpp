#include "jimmy_Core.h"

void StandartHotkeyHandler(std::wstring command_str)
{
    cJSON* commands = cJSON_Parse(STR(command_str.c_str()).c_str());

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

    if (!strcmp(action->valuestring, "KillAll") || !strcmp(action->valuestring, "Pause") || !strcmp(action->valuestring, "Resume") ||
        !strcmp(action->valuestring, "KillOnly") || !strcmp(action->valuestring, "PauseOnly") || !strcmp(action->valuestring, "ResumeOnly"))
    {
        int actionID = 0;

        if (!strcmp(action->valuestring, "KillAll") || !strcmp(action->valuestring, "KillOnly"))
            actionID = PT_KILL;
        else if (!strcmp(action->valuestring, "Pause") || !strcmp(action->valuestring, "PauseOnly"))
            actionID = PT_PAUSE;
        else if (!strcmp(action->valuestring, "Resume") || !strcmp(action->valuestring, "ResumeOnly"))
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
            if (!strcmp(action->valuestring, "KillOnly") || !strcmp(action->valuestring, "PauseOnly") || !strcmp(action->valuestring, "ResumeOnly"))
                ProcessOnly(window, actionID);
            else
                ProcessAll(window, actionID);
        }

        else   if (!strcmp(action->valuestring, "KillAll") || !strcmp(action->valuestring, "Pause") || !strcmp(action->valuestring, "Resume"))
        {
            if (!strcmp(type->valuestring, "Path"))
                ProcessAll(WSTR(subject->valuestring), true, actionID);

            else if (!strcmp(type->valuestring, "ExeName"))
                ProcessAll(WSTR(subject->valuestring), false, actionID);
        }
    }

    else if (!strcmp(action->valuestring, "Topmost") || !strcmp(action->valuestring, "NoTopmost") || !strcmp(action->valuestring, "Minimize"))
    {
        HWND window = NULL;

        if (!strcmp(subject->valuestring, "CurrentWindow"))
            window = GetForegroundWindow();
        else if (!strcmp(subject->valuestring, "UnderMouseWindow"))
        {
            POINT P;
            GetCursorPos(&P);
            window = GetAncestor(WindowFromPoint(P), GA_ROOT);
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
        JimmyGlobalProps_t props = Jimmy_Global_properties.load();

        if (!strcmp(subject->valuestring, "All"))                            
        {
            props.BlockInjected_Keyboard = props.BlockInjected_Keyboard ? false : true;
            props.BlockInjected_Mouse = props.BlockInjected_Mouse ? false : true;
        }

        else if (!strcmp(subject->valuestring, "Mouse"))
            props.BlockInjected_Mouse = props.BlockInjected_Mouse ? false : true;

        else if (!strcmp(subject->valuestring, "Keyboard"))
            props.BlockInjected_Keyboard = props.BlockInjected_Keyboard ? false : true;


        else if (!strcmp(subject->valuestring, "Nothing"))
            props.BlockInjected_Keyboard = props.BlockInjected_Mouse = false;

        Jimmy_Global_properties.store(props);
    }
}

bool LoadConfig()
{

    printf("\n");
    cJSON* config = NULL;
    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();

    {
        char* config_str;
        FILE* fp = NULL;
        size_t size = 0;

        _wfopen_s(&fp, L"Jimmy_config.json", L"rb");
                                                                                              
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
        cJSON* combinations = cJSON_GetObjectItem(config, "Combinations");
        cJSON* InputBlocking = cJSON_GetObjectItem(config, "InputBlocking");
        cJSON* MediaOverlay = cJSON_GetObjectItem(config, "MediaOverlay");
        cJSON* Locker = cJSON_GetObjectItem(config, "Locker");

        if (!combinations || !InputBlocking || !MediaOverlay || !Locker)
        {
            printf("broken config - combination array not found\n");
            return false;
        }
        JimmyGlobalProps_t props = Jimmy_Global_properties.load();
        
        //input filters                                                                   
        if (!strcmp(InputBlocking->valuestring, "All"))
            props.BlockInjected_Keyboard = props.BlockInjected_Mouse = true;
        
        else if (!strcmp(InputBlocking->valuestring, "Mouse"))
            props.BlockInjected_Mouse = true;
        
        else if (!strcmp(InputBlocking->valuestring, "Keyboard"))
            props.BlockInjected_Keyboard = true;
        
        else if (!strcmp(InputBlocking->valuestring, "Nothing"))
            props.BlockInjected_Keyboard = props.BlockInjected_Mouse = false;
        
        //media overlay
        if (!strcmp(MediaOverlay->valuestring, "Enabled"))
            props.MediaOverlayServiceEnabled = true;
        
        else if (!strcmp(MediaOverlay->valuestring, "Disabled"))
            props.MediaOverlayServiceEnabled = false;
        
        //locker
        if (!strcmp(Locker->valuestring, "Enabled"))
            props.LockerServiceEnabled = true;
        
        else if (!strcmp(Locker->valuestring, "Disabled"))
            props.LockerServiceEnabled = false;
        
        Jimmy_Global_properties.store(props);


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

            printf("%s\n", Name->valuestring);

            Hotkey_Settings_t settings;
            settings.Thread_Id = GetCurrentThreadId();
            settings.Msg = WM_HKPP_DEFAULT_CALLBACK_MESSAGE;

            char* strin = cJSON_Print(Actions);
            settings.name = WSTR(strin);
            free(strin);

            settings.Block_Input = GetHKPP_ConstantFromString(BlockInputMode->valuestring);
            settings.Allow_Injected = GetHKPP_ConstantFromString(AllowInjectedI->valuestring);;
            settings.user_callback = [&](Hotkey_Deskriptor desk) -> void {StandartHotkeyHandler(desk.settings.name); };

            VectorEx <key_deskriptor> keys;

            for (int i = 0; i < cJSON_GetArraySize(Keys); i++)
            {
                cJSON* subitem = cJSON_GetArrayItem(Keys, i);
                if (subitem)
                {
                    DWORD key = StrToKey(WSTR(subitem->valuestring));
                    if (key)
                        keys.push_back(key);
                }
            }
            if (keys.size())
                mng->Add(Hotkey_Deskriptor(keys, settings));
        }
    }
    else
    {
        printf("invalid json\n");

    }

    if (config)
        cJSON_Delete(config);

    return true;
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