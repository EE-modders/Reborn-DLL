#include "pch.h"
#include "RebornDLL.h"
#include <iostream>
#include <thread>
#include <string>

unsigned int __stdcall MainThread(void* data) {
    return MainEntry(reinterpret_cast<threadSettings*>(data));
}

bool runsWINE() {
    HMODULE ntdllMod = GetModuleHandle(L"ntdll.dll");

    if (ntdllMod && GetProcAddress(ntdllMod, "wine_get_version"))
        return true;
    else
        return false;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH: {
        // shamelessly copied from https://github.com/EE-modders/Empire-Earth-Library/blob/bb2edcba2ac74b00a997b6536e62be9330331da9/EELib/dllmain.cpp#L60
        // there should be a better way of doing this tough
        DWORD Current_Game_ProcessID = GetCurrentProcessId();
        std::wstring MutexString = L"LibMutexCheck_" + std::to_wstring(Current_Game_ProcessID);
        HANDLE handleMutex = CreateMutexW(NULL, TRUE, MutexString.c_str());

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            return true;
        }
        else {
            char path[MAX_PATH];
            HMODULE hm = NULL;
            GetModuleFileNameA(hm, path, sizeof(path));
            *strrchr(path, '\\') = '\0';
            strcat_s(path, "\\EEReborn.ini");

            /* read data from ini file */
            threadSettings* tData = new threadSettings;

            tData->bDebugMode = GetPrivateProfileIntA("Debug", "DebugMode", 0, path) != 0;
            tData->bWINE = runsWINE();

            tData->resolution.ResPatchType = GetPrivateProfileIntA("Resolution", "ResPatchType", 1, path);
            tData->resolution.xResolution = GetPrivateProfileIntA("Resolution", "xResolution", 800, path);
            tData->resolution.yResolution = GetPrivateProfileIntA("Resolution", "yResolution", 600, path);
            tData->resolution.bForceScenarioEditor = GetPrivateProfileIntA("Resolution", "forceScenarioEditor", 0, path) != 0;

            tData->camera.bCameraPatch = GetPrivateProfileIntA("Camera", "CameraPatch", 0, path) != 0;
            tData->camera.fMaxZHeight = static_cast<float>(GetPrivateProfileIntA("Camera", "MaxZ", 30, path)) * -1.0f;
            tData->camera.fFOV = static_cast<float>(GetPrivateProfileIntA("Camera", "FOV", 60, path)) / 100.0f;
            tData->camera.fFOGDistance = static_cast<float>(GetPrivateProfileIntA("Camera", "Fog", 50, path));
            tData->camera.zoomStyle = GetPrivateProfileIntA("Camera", "Style", 2, path);
            tData->camera.fCameraPitch = static_cast<float>(GetPrivateProfileIntA("Camera", "Pitch", 33, path)) / 100.0f * -1.0f;

            tData->game.maxUnits = GetPrivateProfileIntA("Game", "MaxUnits", 1200, path);

            /*
            _beginthreadex crashes under WINE for some reason
            CreateThread crashes also on Windows
            */
            if (tData->bWINE)
                RebornDLLThread(tData);
            else
                HANDLE threadHandle = (HANDLE)_beginthreadex(0, 0, &MainThread, tData, 0, 0);
            //CreateThread(0, 0, RebornDLLThread, tData, 0, 0); // this crashes for some reason
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        //FreeLibrary(hModule);
        break;
    }
    return true;
}
