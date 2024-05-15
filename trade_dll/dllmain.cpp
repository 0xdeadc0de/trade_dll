#include "pch.h"
#include "stronghold.h"
#include "interface.h"

#pragma comment (lib, "user32.lib")

DWORD WINAPI MainThread(LPVOID param)
{
    // Extract hModule from long pointer void.
    HMODULE hModule = (HMODULE)param;
    
    // Create Settings Window.
    if (!CreateSettingsWindow(hModule))
        return -1;

    // Create Stronghold Thread.
    auto strongholdThread = CreateThread(0, 0, StrongholdThread, hModule, 0, 0);
    if (strongholdThread == NULL)
        return -1;

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Kill Stronghold Thread.
    StrongholdThreadAlive = false;
    WaitForSingleObject(strongholdThread, 60*1000);
    CloseHandle(strongholdThread);

    // Free the memory from DLL and return success.
    FreeLibraryAndExitThread((HMODULE)param, 0);

    return (int)msg.wParam;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID  lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            CreateThread(0, 0, MainThread, hModule, 0, 0);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
