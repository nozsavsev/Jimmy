#define _CRT_SECURE_NO_WARNINGS

#include <SFML/Graphics.hpp>
#include <ShObjIdl.h>

#include "services.h"

float      current_volume;
std::mutex current_volume_mutex;

void help_volume_service()
{
    IMMDeviceEnumerator* deviceEnumerator = NULL;
    IMMDevice* defaultDevice = NULL;
    IAudioEndpointVolume* endpointVolume = NULL;

    CoInitialize(NULL);

    while (1)
    {

        CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);

        deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
        deviceEnumerator->Release();
        deviceEnumerator = NULL;

        defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
        defaultDevice->Release();
        defaultDevice = NULL;

        current_volume_mutex.lock();
        endpointVolume->GetMasterVolumeLevelScalar(&current_volume);
        current_volume *= 100;
        current_volume_mutex.unlock();

        Sleep(100);
    }

    CoUninitialize();
}

void Media_Overlay_service()
{
    std::thread help_volume_s_thread(&help_volume_service);

    pid_protected_mutex.lock();
    pid_protected.push_back(0);
    pid_protected[pid_protected.size() - 1] = GetCurrentProcessId();
    pid_protected_mutex.unlock();

    sf::ContextSettings set;
    sf::RenderWindow window;

    set.antialiasingLevel = 10;
    window.create(sf::VideoMode(110, 25), "", sf::Style::None, set);

    Delete_Taskbar_Icon(window.getSystemHandle());

    window.setPosition(sf::Vector2i(10, 10));
    window.setFramerateLimit(60);

    SetWindowPos(window.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); //make topmost

    SetWindowLong(window.getSystemHandle(), GWL_EXSTYLE, GetWindowLong(window.getSystemHandle(), GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT); //click transparent
    SetLayeredWindowAttributes(window.getSystemHandle(), RGB(0, 0, 0), 0, LWA_COLORKEY);                                                            //click transparent

    window.setVisible(false);

    sf::Event evv;
    sf::RectangleShape rec_1;

    rec_1.setSize(sf::Vector2f(Get_Volume(), 15));
    rec_1.setFillColor(sf::Color(0, 110, 210));
    rec_1.setOutlineColor(sf::Color::White);
    rec_1.setOutlineThickness(-2);

    sf::Clock cl;

    float old_volume = Get_Volume();

    while (1)
    {
        Disable_Overlay();

        old_volume = Get_Volume();

        while (old_volume == Get_Volume()) Sleep(GLB_DEFAULT_TIMEOUT);

        cl.restart();

        window.setVisible(true);
        ShowWindow(window.getSystemHandle(), SW_NORMAL);

        rec_1.setPosition(5, 5);

        while (cl.getElapsedTime().asMilliseconds() < 1500)
        {
            SetWindowPos(window.getSystemHandle(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

            window.pollEvent(evv);

            if (Get_Volume() != old_volume)
            {
                old_volume = Get_Volume();
                cl.restart();
            }

            rec_1.setSize(sf::Vector2f(Get_Volume(), rec_1.getSize().y));

            window.clear(sf::Color(25, 25, 25));

            if (old_volume > 0)
                window.draw(rec_1);

            window.display();
        }
        window.setVisible(false);
    }
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
        {
            ShowWindow(hwndHost, SW_HIDE);
            ShowWindow(hwndHost, SW_MINIMIZE);
            ShowWindow(hwndHost, SW_FORCEMINIMIZE);
        }
}

float Get_Volume()
{
    float curavol = 0;

    current_volume_mutex.lock();
    curavol = current_volume;
    current_volume_mutex.unlock();

    return curavol;
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

    CoUninitialize();

    if (result_volume == new_bolume) return true;
    else                          return false;
}