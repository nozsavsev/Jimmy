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