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
#include <SFML/Graphics.hpp>
#include "Jimmy_Core.h"



struct display_deskriptor { int sx, sy, px, py; };

std::vector <display_deskriptor> ddesk;
BOOL ENudi(HMONITOR p1, HDC hdc, LPRECT lpr, LPARAM p3)
{
    if (lpr)
        ddesk.push_back({ lpr->right - lpr->left,lpr->bottom - lpr->top,lpr->left,lpr->top });

    return true;
}

///*
class color_change_t
{
public:
    sf::Color from;
    sf::Color to;

    sf::Color getVal(float coff)
    {
        return
        {
            (unsigned char)(((float)to.r - (float)from.r) * coff + (float)from.r) ,
            (unsigned char)(((float)to.g - (float)from.g) * coff + (float)from.g) ,
            (unsigned char)(((float)to.b - (float)from.b) * coff + (float)from.b) ,
            (unsigned char)(((float)to.a - (float)from.a) * coff + (float)from.a) ,
        };
    }
};
//*/

#define ANIM_TIME 250.f
#define ANIM_TIME_ALL 300.f

void pcolor(sf::Color c, float coff) { printf("(%d:%d:%d:%d) -> %f\n", c.r, c.g, c.b, c.a, coff); }
void pcolor(float c, float coff) { printf("%f -> %f\n", c, coff); }

void Locker_immproc()
{

    static bool first = true;

    static color_change_t gradi_unlock;
    static color_change_t gradi_lock;
    static sf::Clock unlockAnim;
    static sf::Clock lockAnim;
    static bool justLocked = true;
    static std::vector <sf::RenderWindow*>winvec;
    static sf::Event evt;

    if (first)
    {
        first = false;

        HKPP::Hotkey_Manager::Get_Instance()->Add_Callback([&](int nCode, WPARAM w, LPARAM l, VectorEx<key_deskriptor> keydesk, bool repeated_input) -> bool
            {

                if (keydesk.Contains(Jimmy_Global_properties.Locker_ActivateKey.load()) && !repeated_input && w == WM_KEYDOWN)
                    Jimmy_Global_properties.Locker_IsLocked = true;

                if (keydesk.Contains(Jimmy_Global_properties.Locker_ExitKey.load()) && !repeated_input && w == WM_KEYDOWN)
                    Jimmy_Global_properties.Locker_IsLocked = false;

                return false;
            });


        EnumDisplayMonitors(GetDC(0), NULL, &ENudi, (LPARAM)NULL);

        std::for_each(ddesk.begin(), ddesk.end(), [&](display_deskriptor dd) -> void
            {
                sf::RenderWindow* wpt = new sf::RenderWindow(sf::VideoMode(dd.sx - 1, dd.sy - 1), "", sf::Style::None);
                if (wpt)
                {
                    while (wpt->pollEvent(evt));
                    wpt->clear(sf::Color(50, 180, 50));
                    wpt->display();

                    wpt->setVerticalSyncEnabled(true);
                    SetWindowPos(wpt->getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                    wpt->setPosition(sf::Vector2i(dd.px - 1, dd.py));

                    winvec.push_back(wpt);
                }
            });


        gradi_unlock.from = sf::Color(180, 50, 50);
        gradi_unlock.to = sf::Color(50, 180, 50);

        gradi_lock.from = sf::Color(50, 180, 50);
        gradi_lock.to = sf::Color(180, 50, 50);
    }


    if (Jimmy_Global_properties.Locker_IsLocked || unlockAnim.getElapsedTime().asMilliseconds() < ANIM_TIME_ALL || lockAnim.getElapsedTime().asMilliseconds() < ANIM_TIME_ALL)
    {
        if (justLocked == false)
        {
            std::for_each(winvec.begin(), winvec.end(), [&](sf::RenderWindow* wpt) -> void
                {
                    ShowWindow(wpt->getSystemHandle(), SW_SHOW);
                });
            justLocked = true;
            lockAnim.restart();
        }

        if (!Jimmy_Global_properties.Locker_IsLocked == false)
        {
            unlockAnim.restart();
        }


        {
            HWND hw = winvec[0]->getSystemHandle();
            SetForegroundWindow(hw);
            SetActiveWindow(hw);
        }


        std::for_each(winvec.begin(), winvec.end(), [&](sf::RenderWindow* wpt) -> void
            {
                ShowWindow(wpt->getSystemHandle(), SW_NORMAL);
                while (wpt->pollEvent(evt));

                if (Jimmy_Global_properties.Locker_IsLocked)
                {
                    int cval = lockAnim.getElapsedTime().asMilliseconds();
                    if (cval > ANIM_TIME) cval = ANIM_TIME;

                    wpt->clear(gradi_lock.getVal(cval / ANIM_TIME));
                }
                else
                {
                    int cval = unlockAnim.getElapsedTime().asMilliseconds();
                    if (cval > ANIM_TIME) cval = ANIM_TIME;

                    wpt->clear(gradi_unlock.getVal(cval / ANIM_TIME));
                }

                wpt->display();
            });
    }
    else
    {
        if (justLocked)
        {
            std::for_each(winvec.begin(), winvec.end(), [&](sf::RenderWindow* wpt) -> void { ShowWindow(wpt->getSystemHandle(), SW_HIDE); });
            justLocked = false;
        }

        std::for_each(winvec.begin(), winvec.end(), [&](sf::RenderWindow* wpt) -> void { while (wpt->pollEvent(evt)); });
        Sleep(10);
    }
}