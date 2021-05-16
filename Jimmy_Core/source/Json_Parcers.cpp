#include "jimmy_Core.h"

//deskriptors
#pragma region desks

HWND target_window_t::get()
{
    switch (trg_tp)
    {
    case target_window_type_t::current_window:
        return GetForegroundWindow();
        break;

    case target_window_type_t::undermouse_window:
        POINT P;
        GetCursorPos(&P);
        return GetAncestor(WindowFromPoint(P), GA_ROOT);
        break;
    }
    return nullptr;
}

void process_action_t::set_target(target_window_t wnd)
{
    trg_type = trg_type_t::window; wind = wnd;
};

void process_action_t::set_target(target_process_t prc)
{
    trg_type = trg_type_t::process; proc = prc;
};

void process_action_t::perform()
{
    int action = -1;
    int area = 0;

    switch (act_type)
    {
    case process_action_t::act_type_t::killAll:    area = 1;    action = PT_KILL;   break;
    case process_action_t::act_type_t::killOnly:   area = 0;    action = PT_KILL;   break;

    case process_action_t::act_type_t::pauseAll:   area = 1;    action = PT_PAUSE;  break;
    case process_action_t::act_type_t::pauseOnly:  area = 0;    action = PT_PAUSE;  break;

    case process_action_t::act_type_t::resumeAll:  area = 1;    action = PT_RESUME; break;
    case process_action_t::act_type_t::resumeOnly: area = 0;    action = PT_RESUME; break;
    }

    switch (trg_type)
    {
    case process_action_t::trg_type_t::window:
    {
        if (area)
            ProcessOnly(wind.get(), action);
        else
            ProcessAll_Window(wind.get(), action);
    }
    break;

    case process_action_t::trg_type_t::process:
    {
        bool ptype = (proc.trg_tp == target_process_t::process_type_t::path) ? true : false;
        ProcessAll(proc.process, ptype, action);
    }
    break;
    }
}


void window_action_t::set_target(target_window_t wnd) { target = wnd; };

void window_action_t::perform()
{
    switch (act_type)
    {
    case window_action_t::act_type_t::minimize:
        ShowWindow(target.get(), SW_FORCEMINIMIZE);
        break;
    case window_action_t::act_type_t::topmost:
        SetWindowPos(target.get(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        break;
    case window_action_t::act_type_t::noTopmost:
        SetWindowPos(target.get(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        break;
    }
}



void toggle_input_blocking_t::perform()
{
    {
        switch (trg)
        {
        case toggle_input_blocking_t::target_t::mouse:
            if (act == action_t::toggle)
                Jimmy_Global_properties.BlockInjected_Mouse = Jimmy_Global_properties.BlockInjected_Mouse ? false : true;
            else
                Jimmy_Global_properties.BlockInjected_Mouse = targetVal;
            break;

        case toggle_input_blocking_t::target_t::keyboard:
            if (act == action_t::toggle)
                Jimmy_Global_properties.BlockInjected_Keyboard = Jimmy_Global_properties.BlockInjected_Mouse ? false : true;
            else
                Jimmy_Global_properties.BlockInjected_Keyboard = targetVal;
            break;

        case toggle_input_blocking_t::target_t::all:
            if (act == action_t::toggle)
            {
                Jimmy_Global_properties.BlockInjected_Keyboard = Jimmy_Global_properties.BlockInjected_Mouse ? false : true;
                Jimmy_Global_properties.BlockInjected_Mouse = Jimmy_Global_properties.BlockInjected_Mouse ? false : true;
            }
            else
            {
                Jimmy_Global_properties.BlockInjected_Keyboard = targetVal;
                Jimmy_Global_properties.BlockInjected_Mouse = targetVal;
            }
            break;
        }
    }

}


void action_desk::setAction(process_action_t act)
{
    atype = action_type_t::process;
    p = act;
}

void action_desk::setAction(window_action_t act)
{
    atype = action_type_t::window;
    w = act;
}

void action_desk::setAction(toggle_input_blocking_t act)
{
    atype = action_type_t::input;
    i = act;
}

void action_desk::perform()
{
    switch (atype)
    {
    case action_desk::action_type_t::process:
        p.perform();
        break;
    case action_desk::action_type_t::window:
        w.perform();
        break;
    case action_desk::action_type_t::input:
        i.perform();
        break;
    }
}

#pragma endregion


VectorEx <action_desk> actions;

action_desk GetActionObject(cJSON* command)
{
    cJSON* action = cJSON_GetObjectItem(command, "Action");
    cJSON* status = cJSON_GetObjectItem(command, "Status");
    cJSON* subject = cJSON_GetObjectItem(command, "Subject");
    cJSON* type = cJSON_GetObjectItem(command, "Type");

    if (!action || !status || !subject || !type)
        return {};

    action_desk retval;

    if (!strcmp(action->valuestring, "KillAll") || !strcmp(action->valuestring, "Pause") || !strcmp(action->valuestring, "Resume") ||
        !strcmp(action->valuestring, "KillOnly") || !strcmp(action->valuestring, "PauseOnly") || !strcmp(action->valuestring, "ResumeOnly"))
    {
        process_action_t pa;

        if (!strcmp(action->valuestring, "KillAll"))          pa.act_type = process_action_t::act_type_t::killAll;
        else  if (!strcmp(action->valuestring, "KillOnly"))   pa.act_type = process_action_t::act_type_t::killOnly;
        else  if (!strcmp(action->valuestring, "Pause"))      pa.act_type = process_action_t::act_type_t::pauseAll;
        else  if (!strcmp(action->valuestring, "PauseOnly"))  pa.act_type = process_action_t::act_type_t::pauseOnly;
        else  if (!strcmp(action->valuestring, "Resume"))     pa.act_type = process_action_t::act_type_t::resumeAll;
        else  if (!strcmp(action->valuestring, "ResumeOnly")) pa.act_type = process_action_t::act_type_t::resumeOnly;


        if (!strcmp(type->valuestring, "Concept"))
        {
            target_window_t trgt;

            if (!strcmp(subject->valuestring, "CurrentWindow"))
                trgt.trg_tp = target_window_t::target_window_type_t::current_window;
            else if (!strcmp(subject->valuestring, "UnderMouseWindow"))
                trgt.trg_tp = target_window_t::target_window_type_t::undermouse_window;

            pa.set_target(trgt);
        }

        else if (!strcmp(type->valuestring, "KillAll"))
        {
            target_process_t trgt;
            if (!strcmp(type->valuestring, "Path"))
                trgt.trg_tp = target_process_t::process_type_t::path;
            else if (!strcmp(type->valuestring, "ExeName"))
                trgt.trg_tp = target_process_t::process_type_t::name;

            trgt.process = WSTR(subject->valuestring);

            pa.set_target(trgt);
        }

        retval.setAction(pa);
    }

    else if (!strcmp(action->valuestring, "Topmost") || !strcmp(action->valuestring, "NoTopmost") || !strcmp(action->valuestring, "Minimize"))
    {
        window_action_t wa;

        target_window_t trgt;

        if (!strcmp(subject->valuestring, "CurrentWindow"))
            trgt.trg_tp = target_window_t::target_window_type_t::current_window;
        else if (!strcmp(subject->valuestring, "UnderMouseWindow"))
            trgt.trg_tp = target_window_t::target_window_type_t::undermouse_window;


        wa.set_target(trgt);
        if (!strcmp(action->valuestring, "Topmost"))
            wa.act_type = window_action_t::act_type_t::topmost;

        else if (!strcmp(action->valuestring, "NoTopmost"))
            wa.act_type = window_action_t::act_type_t::noTopmost;

        else if (!strcmp(action->valuestring, "Minimize"))
            wa.act_type = window_action_t::act_type_t::minimize;

        retval.setAction(wa);
    }

    else if (!strcmp(action->valuestring, "ToggleInputBlocking"))
    {
        toggle_input_blocking_t ia;

        ia.act = toggle_input_blocking_t::action_t::toggle;

        if (!strcmp(subject->valuestring, "All"))
            ia.trg = toggle_input_blocking_t::target_t::all;
        else if (!strcmp(subject->valuestring, "Mouse"))
            ia.trg = toggle_input_blocking_t::target_t::mouse;
        else if (!strcmp(subject->valuestring, "Keyboard"))
            ia.trg = toggle_input_blocking_t::target_t::keyboard;

        retval.setAction(ia);
    }
    else
        log("command not found '%s'\n", action->valuestring);

    return retval;
}

bool LoadConfig(DWORD tID)
{
    cJSON* config = NULL;
    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();
    static VectorEx <size_t> uuid_list;

    uuid_list.foreach([&](size_t uuid) -> void {mng->Remove_Hotkey(uuid); });
    uuid_list.clear();

    {
        char* config_str = nullptr;
        FILE* fp = nullptr;
        size_t size = 0;

        _wfopen_s(&fp, L"C:\\Users\\nozsavsev\\Desktop\\Jimmy_Config.json", L"rb");

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
        cJSON* Combinations = cJSON_GetObjectItem(config, "Combinations");
        cJSON* InputBlocking = cJSON_GetObjectItem(config, "InputBlocking");
        cJSON* MediaOverlay = cJSON_GetObjectItem(config, "MediaOverlay");
        cJSON* Locker = cJSON_GetObjectItem(config, "Locker");
        cJSON* AwareList = cJSON_GetObjectItem(config, "Aware");

        if (!Combinations || !InputBlocking || !MediaOverlay || !Locker || !AwareList)
        {
            log("broken config - critical pole not found\nCombinations %s\nInputBlocking %s\nMediaOverlay %s\nLocker %s\nAwareList %s\n",
                Combinations ? "OK" : "NO",
                InputBlocking ? "OK" : "NO",
                MediaOverlay ? "OK" : "NO",
                Locker ? "OK" : "NO",
                AwareList ? "OK" : "NO");

            return false;
        }




        //input filters                                                                   
        if (!strcmp(InputBlocking->valuestring, "All"))
            Jimmy_Global_properties.BlockInjected_Keyboard = Jimmy_Global_properties.BlockInjected_Mouse = true;

        else if (!strcmp(InputBlocking->valuestring, "Mouse"))
            Jimmy_Global_properties.BlockInjected_Mouse = true;

        else if (!strcmp(InputBlocking->valuestring, "Keyboard"))
            Jimmy_Global_properties.BlockInjected_Keyboard = true;

        else if (!strcmp(InputBlocking->valuestring, "Nothing"))
            Jimmy_Global_properties.BlockInjected_Keyboard = Jimmy_Global_properties.BlockInjected_Mouse = false;

        //media overlay
        if (!strcmp(MediaOverlay->valuestring, "True"))
            Jimmy_Global_properties.MediaOverlayService = true;

        else if (!strcmp(MediaOverlay->valuestring, "False"))
            Jimmy_Global_properties.MediaOverlayService = false;

        //locker
        if (!strcmp(Locker->valuestring, "True"))
            Jimmy_Global_properties.LockerService = true;

        else if (!strcmp(Locker->valuestring, "False"))
            Jimmy_Global_properties.LockerService = false;



        for (int i = 0; i < cJSON_GetArraySize(Combinations); i++)
        {
            cJSON* subitem = cJSON_GetArrayItem(Combinations, i);

            if (!subitem)
            {
                log("broken config - combination %d\n", i);
                continue;
            }
            cJSON* Name = cJSON_GetObjectItem(subitem, "Name");
            cJSON* BlockInputMode = cJSON_GetObjectItem(subitem, "BlockInputMode");
            cJSON* AllowInjected = cJSON_GetObjectItem(subitem, "AllowInjected");
            cJSON* Keys = cJSON_GetObjectItem(subitem, "Keys");
            cJSON* Actions = cJSON_GetObjectItem(subitem, "Actions");

            if (!BlockInputMode || !AllowInjected || !Keys || !Actions)
            {
                log("broken config - combination %d\n", i);
                continue;
            }

            Hotkey_Settings_t settings;
            settings.Thread_Id = 0;
            settings.Msg = WM_HKPP_DEFAULT_CALLBACK_MESSAGE;

            settings.name = WSTR(Name->valuestring);

            for (int i = 0; i < cJSON_GetArraySize(Actions); i++)
            {
                cJSON* subitem = cJSON_GetArrayItem(Actions, i);

                if (subitem)
                {
                    actions.push_back(GetActionObject(subitem));
                    settings.userdata = &actions;
                }
            }

            settings.Block_Input = GetHKPP_ConstantFromString(BlockInputMode->valuestring);
            settings.Allow_Injected = GetHKPP_ConstantFromString(AllowInjected->valuestring);;

            settings.user_callback = [&](Hotkey_Deskriptor desk) -> void
            {
                ((VectorEx<action_desk>*)desk.settings.userdata)->foreach([&](action_desk& act) -> void
                    {
                        if (act.uuid == desk.settings.uuid)
                        {
                            act.perform();
                        }
                    });
            };

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
                actions[actions.size() - 1].uuid = mng->Add_Hotkey(Hotkey_Deskriptor(keys, settings));
            log("loaded: '%s'\n", Name->valuestring);
        }
    }
    else
        log("invalid json\n");

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