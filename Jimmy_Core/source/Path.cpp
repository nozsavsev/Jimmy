#include "Jimmy_Core.h"

std::wstring Get_Full_Path(HWND window)
{
    DWORD pID = 0;
    GetWindowThreadProcessId(window, &pID);
    return Get_Full_Path(pID);
}

std::wstring Get_Full_Path(DWORD pID)
{
    std::wstring name = L"";
    DWORD buffSize = 1024;
    wchar_t buffer[1024];

    HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pID);

    if ( handle )
    {
        if ( QueryFullProcessImageNameW(handle, 0, buffer, &buffSize) )
            name = buffer;
        CloseHandle(handle);
    }

    return name;
}

std::wstring Name_From_Path(std::wstring path)
{
    return path.substr(path.find_last_of(L"/\\") + 1);
}