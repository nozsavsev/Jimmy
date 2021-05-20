/*Copyright 2020 Nozdrachev Ilia
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "jimmy_Core.h"

//deskriptors
#pragma region desks

void locker_action_t::Perform()
{
    if (act == Act_Type_t::activate)
        Jimmy_Global_properties.Locker_IsLocked = true;
    else if (act == Act_Type_t::deactivate)
        Jimmy_Global_properties.Locker_IsLocked = false;
}

HWND target_window_t::get()
{
    switch (Trg_tp)
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

void process_Action_t::Set_Target(target_window_t wnd)
{
    Trg_type = Trg_type_t::window; wind = wnd;
};

void process_Action_t::Set_Target(target_process_t prc)
{
    Trg_type = Trg_type_t::process; proc = prc;
};

void process_Action_t::Perform()
{
    int Action = -1;
    int area = 0;

    switch (Act_Type)
    {
    case process_Action_t::Act_Type_t::killAll:    area = 1;    Action = PT_KILL;   break;
    case process_Action_t::Act_Type_t::killOnly:   area = 0;    Action = PT_KILL;   break;

    case process_Action_t::Act_Type_t::pauseAll:   area = 1;    Action = PT_PAUSE;  break;
    case process_Action_t::Act_Type_t::pauseOnly:  area = 0;    Action = PT_PAUSE;  break;

    case process_Action_t::Act_Type_t::resumeAll:  area = 1;    Action = PT_RESUME; break;
    case process_Action_t::Act_Type_t::resumeOnly: area = 0;    Action = PT_RESUME; break;
    }

    switch (Trg_type)
    {
    case process_Action_t::Trg_type_t::window:
    {
        if (area)
            Process_Only(wind.get(), Action);
        else
            Process_All_Window(wind.get(), Action);
    }
    break;

    case process_Action_t::Trg_type_t::process:
    {
        bool ptype = (proc.Trg_tp == target_process_t::process_type_t::path) ? true : false;
        Process_All(proc.process, ptype, Action);
    }
    break;
    }
}


void window_Action_t::Set_Target(target_window_t wnd) { target = wnd; };

void window_Action_t::Perform()
{
    switch (Act_Type)
    {
    case window_Action_t::Act_Type_t::minimize:
        ShowWindow(target.get(), SW_FORCEMINIMIZE);
        break;
    case window_Action_t::Act_Type_t::topmost:
        SetWindowPos(target.get(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        break;
    case window_Action_t::Act_Type_t::noTopmost:
        SetWindowPos(target.get(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        break;
    }
}



void toggle_input_blocking_t::Perform()
{
    {
        switch (Trg)
        {
        case toggle_input_blocking_t::target_t::mouse:
            if (Act == Action_t::toggle)
                Jimmy_Global_properties.Block_Injected_Mouse = Jimmy_Global_properties.Block_Injected_Mouse ? false : true;
            else
                Jimmy_Global_properties.Block_Injected_Mouse = Target_Val;
            break;

        case toggle_input_blocking_t::target_t::keyboard:
            if (Act == Action_t::toggle)
                Jimmy_Global_properties.Block_Injected_Keyboard = Jimmy_Global_properties.Block_Injected_Mouse ? false : true;
            else
                Jimmy_Global_properties.Block_Injected_Keyboard = Target_Val;
            break;

        case toggle_input_blocking_t::target_t::all:
            if (Act == Action_t::toggle)
            {
                Jimmy_Global_properties.Block_Injected_Keyboard = Jimmy_Global_properties.Block_Injected_Mouse ? false : true;
                Jimmy_Global_properties.Block_Injected_Mouse = Jimmy_Global_properties.Block_Injected_Mouse ? false : true;
            }
            else
            {
                Jimmy_Global_properties.Block_Injected_Keyboard = Target_Val;
                Jimmy_Global_properties.Block_Injected_Mouse = Target_Val;
            }
            break;
        }
    }

}


void Action_desk::setAction(process_Action_t Act)
{
    atype = Action_type_t::process;
    p = Act;
}

void Action_desk::setAction(window_Action_t Act)
{
    atype = Action_type_t::window;
    w = Act;
}

void Action_desk::setAction(toggle_input_blocking_t Act)
{
    atype = Action_type_t::input;
    i = Act;
}

void Action_desk::setAction(locker_action_t Act)
{
    atype = Action_type_t::locker;
    l = Act;
}

void Action_desk::Perform()
{
    switch (atype)
    {
    case Action_desk::Action_type_t::process:
        p.Perform();
        break;
    case Action_desk::Action_type_t::window:
        w.Perform();
        break;
    case Action_desk::Action_type_t::input:
        i.Perform();
        break;
    case Action_desk::Action_type_t::locker:
        l.Perform();
        break;
    }
}

#pragma endregion

VectorEx <acta_t> actlist;

Action_desk Get_Action_Object(cJSON* command)
{
    cJSON* Action = cJSON_GetObjectItem(command, "Action");
    cJSON* status = cJSON_GetObjectItem(command, "Status");
    cJSON* subject = cJSON_GetObjectItem(command, "Subject");
    cJSON* type = cJSON_GetObjectItem(command, "Type");

    if (!Action || !status || !subject || !type)
        return {};

    Action_desk retval;

    if (!strcmp(Action->valuestring, "KillAll") || !strcmp(Action->valuestring, "PauseAll") || !strcmp(Action->valuestring, "Resume") ||
        !strcmp(Action->valuestring, "KillOnly") || !strcmp(Action->valuestring, "PauseOnly") || !strcmp(Action->valuestring, "ResumeOnly"))
    {
        process_Action_t pa;

        if (!strcmp(Action->valuestring, "KillAll"))          pa.Act_Type = process_Action_t::Act_Type_t::killAll;
        else  if (!strcmp(Action->valuestring, "KillOnly"))   pa.Act_Type = process_Action_t::Act_Type_t::killOnly;
        else  if (!strcmp(Action->valuestring, "PauseAll"))      pa.Act_Type = process_Action_t::Act_Type_t::pauseAll;
        else  if (!strcmp(Action->valuestring, "PauseOnly"))  pa.Act_Type = process_Action_t::Act_Type_t::pauseOnly;
        else  if (!strcmp(Action->valuestring, "Resume"))     pa.Act_Type = process_Action_t::Act_Type_t::resumeAll;
        else  if (!strcmp(Action->valuestring, "ResumeOnly")) pa.Act_Type = process_Action_t::Act_Type_t::resumeOnly;


        if (!strcmp(type->valuestring, "Concept"))
        {
            target_window_t Trgt;

            if (!strcmp(subject->valuestring, "CurrentWindow"))
                Trgt.Trg_tp = target_window_t::target_window_type_t::current_window;
            else if (!strcmp(subject->valuestring, "UnderMouseWindow"))
                Trgt.Trg_tp = target_window_t::target_window_type_t::undermouse_window;

            pa.Set_Target(Trgt);
        }

        else
        {
            target_process_t Trgt;
            if (!strcmp(type->valuestring, "Path"))
                Trgt.Trg_tp = target_process_t::process_type_t::path;
            else if (!strcmp(type->valuestring, "ExeName"))
                Trgt.Trg_tp = target_process_t::process_type_t::name;

            Trgt.process = WSTR(subject->valuestring);

            pa.Set_Target(Trgt);
        }

        retval.setAction(pa);
    }

    else if (!strcmp(Action->valuestring, "Topmost") || !strcmp(Action->valuestring, "NoTopmost") || !strcmp(Action->valuestring, "Minimize"))
    {
        window_Action_t wa;

        target_window_t Trgt;

        if (!strcmp(subject->valuestring, "CurrentWindow"))
            Trgt.Trg_tp = target_window_t::target_window_type_t::current_window;
        else if (!strcmp(subject->valuestring, "UnderMouseWindow"))
            Trgt.Trg_tp = target_window_t::target_window_type_t::undermouse_window;


        wa.Set_Target(Trgt);
        if (!strcmp(Action->valuestring, "Topmost"))
            wa.Act_Type = window_Action_t::Act_Type_t::topmost;

        else if (!strcmp(Action->valuestring, "NoTopmost"))
            wa.Act_Type = window_Action_t::Act_Type_t::noTopmost;

        else if (!strcmp(Action->valuestring, "Minimize"))
            wa.Act_Type = window_Action_t::Act_Type_t::minimize;

        retval.setAction(wa);
    }

    else if (!strcmp(Action->valuestring, "ToggleInputBlocking"))
    {
        toggle_input_blocking_t ia;

        ia.Act = toggle_input_blocking_t::Action_t::toggle;

        if (!strcmp(subject->valuestring, "All"))
            ia.Trg = toggle_input_blocking_t::target_t::all;
        else if (!strcmp(subject->valuestring, "Mouse"))
            ia.Trg = toggle_input_blocking_t::target_t::mouse;
        else if (!strcmp(subject->valuestring, "Keyboard"))
            ia.Trg = toggle_input_blocking_t::target_t::keyboard;

        retval.setAction(ia);
    }
    else if (!strcmp(Action->valuestring, "ActivateLocker"))
    {
        locker_action_t a;
        a.act = locker_action_t::Act_Type_t::activate;
        retval.setAction(a);
    }
    else if (!strcmp(Action->valuestring, "DeActivateLocker"))
    {
        locker_action_t a;
        a.act = locker_action_t::Act_Type_t::deactivate;
        retval.setAction(a);
    }
    else
        Log("command not found '%s'\n", Action->valuestring);

    return retval;
}

bool Load_Config(DWORD tID)
{

#pragma region File
    cJSON* config = NULL;
    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();
    static VectorEx <size_t> uuid_list;

    uuid_list.Foreach([&](size_t uuid) -> void {mng->Remove_Hotkey(uuid); });
    uuid_list.clear();

    {
        char* config_str = nullptr;
        FILE* fp = nullptr;
        size_t size = 0;

        _wfopen_s(&fp, L"C:\\users\\nozsavsev\\desktop\\Jimmy_Config.json", L"rb");

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
#pragma endregion

#pragma region first init  
    if (config)
    {
        cJSON* Combinations = cJSON_GetObjectItem(config, "Combinations");
        cJSON* InputBlocking = cJSON_GetObjectItem(config, "InputBlocking");
        cJSON* MediaOverlay = cJSON_GetObjectItem(config, "MediaOverlay");

        cJSON* LockerService = cJSON_GetObjectItem(config, "LockerService");
        cJSON* Locker = cJSON_GetObjectItem(config, "Locker");
        cJSON* LockerActivateKey = cJSON_GetObjectItem(config, "LockerActivateKey");
        cJSON* LockerExitKey = cJSON_GetObjectItem(config, "LockerExitKey");



        if (!Combinations || !InputBlocking || !MediaOverlay || !LockerService)
        {
            Log("broken config - critical pole not found\nCombinations %s\nInputBlocking %s\nMediaOverlay %s\nLocker %s\n",
                Combinations ? "OK" : "NO",
                InputBlocking ? "OK" : "NO",
                MediaOverlay ? "OK" : "NO",
                Locker ? "OK" : "NO");

            return false;
        }


        //input filters                                                                   
        if (!strcmp(InputBlocking->valuestring, "All"))
            Jimmy_Global_properties.Block_Injected_Keyboard = Jimmy_Global_properties.Block_Injected_Mouse = true;

        else if (!strcmp(InputBlocking->valuestring, "Mouse"))
            Jimmy_Global_properties.Block_Injected_Mouse = true;

        else if (!strcmp(InputBlocking->valuestring, "Keyboard"))
            Jimmy_Global_properties.Block_Injected_Keyboard = true;

        else if (!strcmp(InputBlocking->valuestring, "Nothing"))
            Jimmy_Global_properties.Block_Injected_Keyboard = Jimmy_Global_properties.Block_Injected_Mouse = false;

        //media overlay
        if (!strcmp(MediaOverlay->valuestring, "True"))
            Jimmy_Global_properties.Media_Overlay_Service = true;

        else if (!strcmp(MediaOverlay->valuestring, "False"))
            Jimmy_Global_properties.Media_Overlay_Service = false;

        //locker
        if (!strcmp(LockerService->valuestring, "True"))
            Jimmy_Global_properties.Locker_Service = true;

        else if (!strcmp(LockerService->valuestring, "False"))
            Jimmy_Global_properties.Locker_Service = false;


        if (!strcmp(Locker->valuestring, "True"))
            Jimmy_Global_properties.Locker_IsLocked = true;

        else if (!strcmp(Locker->valuestring, "False"))
            Jimmy_Global_properties.Locker_IsLocked = false;

        Jimmy_Global_properties.Locker_ActivateKey = StrToKey(WSTR(LockerActivateKey->valuestring));
        Jimmy_Global_properties.Locker_ExitKey = StrToKey(WSTR(LockerExitKey->valuestring));
#pragma endregion


        for (int i = 0; i < cJSON_GetArraySize(Combinations); i++)
        {
            cJSON* subitem = cJSON_GetArrayItem(Combinations, i);

            if (!subitem)
            {
                Log("broken config - combination %d\n", i);
                continue;
            }
            cJSON* Name = cJSON_GetObjectItem(subitem, "Name");
            cJSON* BlockInputMode = cJSON_GetObjectItem(subitem, "BlockInputMode");
            cJSON* AllowInjected = cJSON_GetObjectItem(subitem, "AllowInjected");
            cJSON* Keys = cJSON_GetObjectItem(subitem, "Keys");
            cJSON* Actions_json = cJSON_GetObjectItem(subitem, "Actions");

            if (!BlockInputMode || !AllowInjected || !Keys || !Actions_json)
            {
                Log("broken config - combination %d\n", i);
                continue;
            }

            Hotkey_Settings_t settings;
            settings.Thread_Id = 0;
            settings.Msg = WM_HKPP_DEFAULT_CALLBACK_MESSAGE;

            settings.Name = WSTR(Name->valuestring);

            if (cJSON_GetArraySize(Actions_json))
            {
                actlist.push_back(acta_t());

                for (int i = 0; i < cJSON_GetArraySize(Actions_json); i++)
                {
                    cJSON* subitem = cJSON_GetArrayItem(Actions_json, i);

                    if (subitem)
                    {
                        actlist[actlist.size() - 1].actions.push_back(Get_Action_Object(subitem));
                        settings.userdata = &actlist;
                    }
                }
            }

            settings.Block_Input = Get_HKPP_Constant_From_String(BlockInputMode->valuestring);
            settings.Allow_Injected = Get_HKPP_Constant_From_String(AllowInjected->valuestring);;

            settings.User_Callback = [&](Hotkey_Deskriptor desk) -> void
            {
                ((VectorEx<acta_t>*)desk.settings.userdata)->Foreach([&](acta_t& Acta) -> void
                    {
                        if (Acta.Uuid == desk.settings.Uuid)
                        {
                            Acta.actions.Foreach([&](Action_desk& ac) -> void { ac.Perform(); });
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
                actlist[actlist.size() - 1].Uuid = mng->Add_Hotkey(Hotkey_Deskriptor(keys, settings));
            Log("loaded: '%s'\n", Name->valuestring);
        }
    }
    else
        Log("invalid json\n");

    if (config)
        cJSON_Delete(config);

    return true;
}

int Get_HKPP_Constant_From_String(char* str)
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