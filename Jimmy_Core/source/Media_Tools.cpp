#include "Jimmy_Core.h"

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