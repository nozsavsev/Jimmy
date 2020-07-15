#include "services.h"

void Wait_For_Key_Release(int key)
{
    while (isKeyPressed(key))
        Sleep(GLB_DEFAULT_TIMEOUT);
}

void Wait_For_Key_Down(int key)
{
    while (!isKeyPressed(key))
        Sleep(GLB_DEFAULT_TIMEOUT);
}

bool isKeyPressed(int key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}