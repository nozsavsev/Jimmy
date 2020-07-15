#include <SFML/Graphics.hpp>
#include "services.h"

std::mutex agersive_topmost_window_handle_mutex;
HWND       agersive_topmost_window_handle;

std::mutex         jimmy_hwnd_protected_mutex;
std::vector <HWND> jimmy_hwnd_protected;

bool is_hwnd_protected(HWND window);
bool Is_Window_Topmost(HWND hWnd);
void Agresive_Topmost();

void Window_Manage_service()
{
    pid_protected_mutex.lock();
    pid_protected.push_back(0);
    pid_protected[pid_protected.size() - 1] = GetCurrentProcessId();
    pid_protected_mutex.unlock();

    std::thread Agresive_Topmost_thread(&Agresive_Topmost);

    HWND hidden_window = NULL;

    while (1)
    {
        Wait_For_Key_Down(GLB_ACTIVATE_KEY);

        exit_state_mutex.lock();
        if (exit_state)
        {
            exit_state_mutex.unlock();
            ShowWindow(hidden_window, SW_NORMAL);
            return;
        }
        else exit_state_mutex.unlock();

        if (isKeyPressed(GLB_ACTIVATE_KEY))
        {

            if (isKeyPressed(WMS_TOPMOST_KEY))
            {
                HWND temp_handle = GetForegroundWindow();

                if (!is_hwnd_protected(temp_handle))
                {
                    if (Is_Window_Topmost(temp_handle))
                    {
                        agersive_topmost_window_handle_mutex.lock();
                        agersive_topmost_window_handle = temp_handle;
                        agersive_topmost_window_handle_mutex.unlock();
                    }

                    SetWindowPos(temp_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                }

                Wait_For_Key_Release(WMS_TOPMOST_KEY);
            }

            if (isKeyPressed(WMS_INFO_TP_KEY))
            {
                HWND temp_handle = GetForegroundWindow();

                if (!is_hwnd_protected(temp_handle))
                {
                    if (Is_Window_Topmost(temp_handle))
                    {
                        agersive_topmost_window_handle_mutex.lock();
                        agersive_topmost_window_handle = temp_handle;
                        agersive_topmost_window_handle_mutex.unlock();
                    }

                    SetWindowPos(temp_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                    Set_Transparency(temp_handle, 60);
                }
                Wait_For_Key_Release(WMS_INFO_TP_KEY);
            }

            if (isKeyPressed(WMS_DIS_TOPMOST_KEY))
            {
                HWND temp_handle = GetForegroundWindow();

                if (!is_hwnd_protected(temp_handle))
                {

                    agersive_topmost_window_handle_mutex.lock();

                    if (temp_handle == agersive_topmost_window_handle)
                    {
                        agersive_topmost_window_handle = NULL;
                    }

                    agersive_topmost_window_handle_mutex.unlock();

                    SetWindowPos(temp_handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                    Set_Transparency(temp_handle, (char)255);
                }

                Wait_For_Key_Release(WMS_DIS_TOPMOST_KEY);
            }

            if (!isKeyPressed(GLB_NON_RECOVERABLE_SECONDARY_ACTIVATE_KEY))
            {
                if (isKeyPressed(WMS_HIDE_KEY))
                {
                    HWND temp_handle = GetForegroundWindow();

                    if (!is_hwnd_protected(temp_handle))
                    {
                        if (hidden_window)
                            ShowWindow(hidden_window, SW_SHOW);

                        hidden_window = temp_handle;
                        ShowWindow(temp_handle, SW_HIDE);
                    }

                    Wait_For_Key_Release(WMS_HIDE_KEY);
                }
            }
            else
                if (isKeyPressed(WMS_HIDE_KEY))
                {
                    HWND temp_handle = GetForegroundWindow();

                    if (!is_hwnd_protected(temp_handle))
                    {
                        EnableWindow(temp_handle, false);
                        ShowWindow(temp_handle, SW_HIDE);
                    }

                    Wait_For_Key_Release(WMS_SHOW_KEY);
                }

            if (isKeyPressed(WMS_SHOW_KEY))
            {
                ShowWindow(hidden_window, SW_SHOW);
                hidden_window = NULL;

                Wait_For_Key_Release(WMS_SHOW_KEY);
            }


            if (isKeyPressed(WMS_ACTIVATE_KEY))
            {
                HWND temp_handle = GetForegroundWindow();

                if (!is_hwnd_protected(temp_handle))
                {
                    EnableWindow(temp_handle, true);
                }

                Wait_For_Key_Release(WMS_ACTIVATE_KEY);
            }


            if (isKeyPressed(WMS_MINIMIZE_KEY))
            {
                HWND temp_handle = GetForegroundWindow();

                if (!is_hwnd_protected(temp_handle))
                {
                    EnableWindow(temp_handle, false);
                    ShowWindow(temp_handle, SW_MINIMIZE);
                    ShowWindow(temp_handle, SW_FORCEMINIMIZE);
                }

                Wait_For_Key_Release(WMS_MINIMIZE_KEY);
            }
        }
    }
}

std::vector <MONITORINFO> monifo_vector;

BOOL MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    MONITORINFO info;

    info.cbSize = sizeof(info);

    if (GetMonitorInfo(hMonitor, &info))
    {
        monifo_vector.push_back({ 0 });

        monifo_vector[monifo_vector.size() - 1] = info;
    }

    return TRUE;
}

void Locker()
{
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    pid_protected_mutex.lock();
    pid_protected.push_back(0);
    pid_protected[pid_protected.size() - 1] = GetCurrentProcessId();
    pid_protected_mutex.unlock();

    sf::ContextSettings set;
    set.antialiasingLevel = 10;

    sf::CircleShape sir;
    sf::RectangleShape rec;

    rec.setSize(sf::Vector2f(110, 110));
    rec.setOrigin(55, 55);
    rec.setPosition(1920 / 2, 1080 / 2);
    rec.setFillColor(sf::Color::Transparent);
    rec.setOutlineColor(sf::Color(0, 0, 0, 0));
    rec.setOutlineThickness(-2);

    sir.setRadius(100);
    sir.setOrigin(100, 100);
    sir.setPosition(1920 / 2, 1080 / 2);

    sir.setOutlineThickness(-2);
    sir.setFillColor(sf::Color::Transparent);
    sir.setOutlineColor(sf::Color(0, 0, 0, 0));

    sf::Event evv;

    while (1)
    {
        Wait_For_Key_Down(VK_RCONTROL);

        float scd = 0;
        float ssf = -1;

        float rot_1 = 0;
        float rot_2 = 0;

        float r_spd = 0.8f;

        sf::RenderWindow window[25];

        agersive_topmost_window_handle_mutex.lock();
        HWND agresive_topmost_backup = agersive_topmost_window_handle;
        agersive_topmost_window_handle_mutex.unlock();

        for (int i = 0; i < monifo_vector.size(); i++)
        {
            int x_szz = monifo_vector[i].rcMonitor.right - monifo_vector[i].rcMonitor.left;
            int y_szz = monifo_vector[i].rcMonitor.bottom - monifo_vector[i].rcMonitor.top;

            window[i].create(sf::VideoMode(x_szz + 1, y_szz + 1), "", sf::Style::None, set);

            jimmy_hwnd_protected_mutex.lock();
            jimmy_hwnd_protected.push_back(window[i].getSystemHandle());
            jimmy_hwnd_protected_mutex.unlock();

            SetWindowPos(window[i].getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

            window[i].setPosition(sf::Vector2i(monifo_vector[i].rcMonitor.left, monifo_vector[i].rcMonitor.top));

            window[i].setFramerateLimit(60);
        }

        while (sir.getOutlineColor().a < 255)
        {
            SetActiveWindow(window[0].getSystemHandle());
            SetForegroundWindow(window[0].getSystemHandle());

            sir.setOutlineColor(sf::Color(
                sir.getOutlineColor().r,
                sir.getOutlineColor().g,
                sir.getOutlineColor().b,
                sir.getOutlineColor().a + 5
            ));

            rec.setOutlineColor(sf::Color(
                rec.getOutlineColor().r,
                rec.getOutlineColor().g,
                rec.getOutlineColor().b,
                rec.getOutlineColor().a + 5
            ));

            if (scd >= 90) ssf = -r_spd;
            if (scd <= 0)  ssf = r_spd;

            sir.setRadius(scd);
            sir.setOrigin(scd, scd);

            scd += ssf;

            for (int i = 0; i < monifo_vector.size(); i++)
            {
                window[i].pollEvent(evv);

                window[i].clear(sf::Color::White);
                window[i].draw(sir);

                rec.setRotation(rot_1++);
                rot_1 += r_spd;
                window[i].draw(rec);

                rec.setRotation(rot_2--);
                rot_2 -= r_spd;
                window[i].draw(rec);
            }

            for (int i = 0; i < monifo_vector.size(); i++)
                window[i].display();
        }

        while (!isKeyPressed('Q'))
        {

            HWND temp_handle = GetForegroundWindow();

            if (!is_hwnd_protected(temp_handle))
            {
                ShowWindow(temp_handle, SW_FORCEMINIMIZE);
                SetForegroundWindow(window[0].getSystemHandle());
            }

            if (scd >= 90) ssf = -r_spd;
            if (scd <= 40) ssf = r_spd;

            scd += ssf;

            sir.setRadius(scd);
            sir.setOrigin(scd, scd);

            for (int i = 0; i < monifo_vector.size(); i++)
            {
                window[i].pollEvent(evv);

                window[i].clear(sf::Color::White);


                window[i].draw(sir);

                rec.setRotation(rot_1);
                rot_1 += r_spd;
                window[i].draw(rec);

                rec.setRotation(rot_2);
                rot_2 -= r_spd;
                window[i].draw(rec);
            }

            for (int i = 0; i < monifo_vector.size(); i++)
                window[i].display();
        }

        while (sir.getOutlineColor().a > 0)
        {

            sir.setOutlineColor(sf::Color(
                sir.getOutlineColor().r,
                sir.getOutlineColor().g,
                sir.getOutlineColor().b,
                sir.getOutlineColor().a - 5
            ));

            rec.setOutlineColor(sf::Color(
                rec.getOutlineColor().r,
                rec.getOutlineColor().g,
                rec.getOutlineColor().b,
                rec.getOutlineColor().a - 5
            ));

            if (scd >= 90) ssf = -1;
            if (scd <= 0) ssf = 1;

            sir.setRadius(scd);
            sir.setOrigin(scd, scd);

            rec.setRotation(rot_1++);
            rec.setRotation(rot_2--);

            scd += ssf;

            for (int i = 0; i < monifo_vector.size(); i++)
            {
                window[i].pollEvent(evv);

                window[i].clear(sf::Color::White);


                window[i].draw(sir);

                rec.setRotation(rot_1++);
                window[i].draw(rec);

                rec.setRotation(rot_2--);
                window[i].draw(rec);
            }

            for (int i = 0; i < monifo_vector.size(); i++)
                window[i].display();
        }

        for (int i = 0; i < monifo_vector.size(); i++)
        {
            window[i].setVisible(false);
        }

        for (int i = 0; i < monifo_vector.size(); i++)
        {
            window[i].close();

            jimmy_hwnd_protected_mutex.lock();

            for (int i = 0; i < jimmy_hwnd_protected.size(); i++)
            {

                if (window[i].getSystemHandle() == jimmy_hwnd_protected[i])
                    jimmy_hwnd_protected.erase(jimmy_hwnd_protected.begin() + i);
            }

            jimmy_hwnd_protected_mutex.unlock();

        }

        agersive_topmost_window_handle_mutex.lock();
        agersive_topmost_window_handle = agresive_topmost_backup;
        agersive_topmost_window_handle_mutex.unlock();
    }
}

bool is_hwnd_protected(HWND window)
{
    jimmy_hwnd_protected_mutex.lock();

    for (int i = 0; i < jimmy_hwnd_protected.size(); i++)
    {
        if (jimmy_hwnd_protected[i] == window)
        {
            jimmy_hwnd_protected_mutex.unlock();
            return true;
        }
    }

    jimmy_hwnd_protected_mutex.unlock();

    return false;
}

void Set_Transparency(HWND hwnd, char Transperancy)
{
    SetWindowLongW(hwnd, GWL_EXSTYLE, GetWindowLongW(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, Transperancy, LWA_ALPHA);
}

void Agresive_Topmost()
{
    pid_protected_mutex.lock();
    pid_protected.push_back(0);
    pid_protected[pid_protected.size() - 1] = GetCurrentProcessId();
    pid_protected_mutex.unlock();

    while (1)
    {

        agersive_topmost_window_handle_mutex.lock();

        if (IsWindow(agersive_topmost_window_handle))//if handle != NULL
        {
            SetWindowPos(agersive_topmost_window_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            SetWindowPos(agersive_topmost_window_handle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        }

        agersive_topmost_window_handle_mutex.unlock();

        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
}

bool Is_Window_Topmost(HWND hWnd)
{
    return (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST;
}