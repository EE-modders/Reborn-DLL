#include "pch.h"
#include "RebornDLL.h"
#include <Windows.h>
#include <iostream>
#include <sstream>

/* memory values for EEC GOG */

DWORD MaxZHeightAddr    = 0x42C700; // float
DWORD CurrZHeightAddr   = 0x518d40; // float
DWORD LastZHeightAddr   = 0x518D08; // float
DWORD ZoomStateAddr     = 0x518DB0; // 0.0: fully zoomed in; 1.0: fully zoomed out | float
DWORD ZoomStyleAddr     = 0x518dc4; // 0, 1, 2 | int
DWORD FOVAddr           = 0x51328c; // float
DWORD FOGDistanceAddr   = 0x42c704; // float
DWORD MaxPitchAddr      = 0x513284; // 0.0: 0d pitch; -1.0: 90d pitch | float
DWORD CurrPitchAddr     = 0x5183c8; // float

DWORD resSwitchCheckAddr = 0x25FAC2; // 2 bytes containing JE 15

DWORD versionStrPtrAddr = 0x1D16FB; // version string pointer
DWORD versionStrStatic  = 0x4A9030; // static version string

DWORD playerMapIsLoaded = 0x517BB8 + 0x7C4; // value is > 0 when map is loaded and running | float

// all following values are int
DWORD xResSettingsAddr  = 0x5193FC; // xRes set in the ingame settings
DWORD yResSettingsAddr  = 0x5193F8;
DWORD bitSettingsAddr   = 0x5193F4;

DWORD xResStartupAddr   = 0x137523; // initial x-resolution
DWORD yResStartupAddr   = 0x137528; // initial y-resolution
DWORD yResBINKAddr      = 0x13753C; // y-axis resolution limit for BINK videos

DWORD xResStartupMainMenuAddr = 0x138B3D; // main menu resolution set when starting
DWORD yResStartupMainMenuAddr = 0x138B38;

DWORD xResMainMenuAddr  = 0x25FACE; // main menu resolution when not starting up
DWORD yResMainMenuAddr  = 0x25FAC9;

DWORD xResScenarioEditorAddr = 0x2601EB; // scenario editor resoluton
DWORD yResScenarioEditorAddr = 0x2601E6;
// ---

// EE Objects

DWORD EEDataPtrAddr = 0x517BB8; // this will be non 00 when EEData is loaded

// ---

memoryPTR maxUnitsPTR = {
    EEDataPtrAddr,
    1,
    { 0x2EC }
};

/*###################################*/

// reading and writing stuff / helper functions and other crap

/* update memory protection and read with memcpy */
void protectedCpy(void* dest, void* src, int n) {
    DWORD oldProtect = 0;
    VirtualProtect(dest, n, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dest, src, n);
    VirtualProtect(dest, n, oldProtect, &oldProtect);
}
/* read from address into read buffer of length len */
bool readBytes(void* read_addr, void* read_buffer, int len) {
    // compile with "/EHa" to make this work
    // see https://stackoverflow.com/questions/16612444/catch-a-memory-access-violation-in-c
    try {
        protectedCpy(read_buffer, read_addr, len);
        return true;
    }
    catch (...) {
        return false;
    }
}
/* write patch of length len to destination address */
void writeBytes(void* dest_addr, void* patch, int len) {
    protectedCpy(dest_addr, patch, len);
}

// fiddle around with pointers and other address stuff

HMODULE getBaseAddress() {
    return GetModuleHandle(NULL);
}

DWORD* getAbsAddress(DWORD appl_addr) {
    return (DWORD*)((DWORD)getBaseAddress() + appl_addr);
}
DWORD* tracePointer(memoryPTR* patch) {
    DWORD* location = getAbsAddress(patch->base_address);

    for (int i = 0; i < patch->total_offsets; i++) {
        location = (DWORD*)(*location + patch->offsets[i]);
    }
    return location;
}

// other helper functions

void showMessage(void* val) {
    std::cout << "DEBUG: " << val << std::endl;
}
void showMessage(float val) {
    std::cout << "DEBUG: " << val << std::endl;
}
void showMessage(int val) {
    std::cout << "DEBUG: " << val << std::endl;
}
void showMessage(LPCSTR val) {
    std::cout << "DEBUG: " << val << std::endl;
}
void showMessage(DWORD val) {
    std::cout << "DEBUG: " << val << std::endl;
}

/*###################################*/

void startupMessage() {
    std::cout << "RebornDLL by zocker_160 & EnergyCube - Version: v" << version_maj << "." << version_min << std::endl;
    std::cout << "Debug mode enabled!\n";
}

void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

void getResolution(int& x, int& y, resolutionSettings* tSet) {

    switch (tSet->ResPatchType) {
    case RES_CUSTOM:
        x = tSet->xResolution;
        y = tSet->yResolution;
        break;
    case RES_GAME:
        x = *(int*)(getAbsAddress(xResSettingsAddr));
        y = *(int*)(getAbsAddress(yResSettingsAddr));
        break;
    case RES_WIN:
        GetDesktopResolution(x, y);
        break;
    default:
        tSet->ResPatchType = RES_GAME;
        getResolution(x, y, tSet);
        break;
    }
}

void setResolutions(resolutionSettings* tSet) {
    showMessage("Pre Resolution Settings");

    if (tSet->ResPatchType == RES_DISABLED) {
        showMessage("Skipped...");
        return;
    }

    int xWantedRes, yWantedRes, xCurrentRes, yCurrentRes;
    bool bResMismatchX, bResMismatchY;


    getResolution(xWantedRes, yWantedRes, tSet);
    xCurrentRes = *(int*)(getAbsAddress(xResStartupAddr));
    yCurrentRes = *(int*)(getAbsAddress(yResStartupAddr));
    
    bResMismatchX = xWantedRes != xCurrentRes;
    bResMismatchY = yWantedRes != yCurrentRes;
    if (bResMismatchX || bResMismatchY) {
        std::cout << "Resolutions don't match" << std::endl;
        std::cout << "Current : " << xCurrentRes << "x" << yCurrentRes << std::endl;
        std::cout << "Wanted : " << xWantedRes << "x" << yWantedRes << std::endl;
        std::cout << "Set new resolution (type : " << tSet->ResPatchType << ")" << std::endl;

        writeBytes(getAbsAddress(xResStartupAddr), &xWantedRes, 4);
        writeBytes(getAbsAddress(yResStartupAddr), &yWantedRes, 4);
        writeBytes(getAbsAddress(yResBINKAddr), &yWantedRes, 4);

        writeBytes(getAbsAddress(xResStartupMainMenuAddr), &xWantedRes, 4);
        writeBytes(getAbsAddress(yResStartupMainMenuAddr), &yWantedRes, 4);

        writeBytes(getAbsAddress(xResMainMenuAddr), &xWantedRes, 4);
        writeBytes(getAbsAddress(yResMainMenuAddr), &yWantedRes, 4);

        if (tSet->bForceScenarioEditor) {
            showMessage("Forced Scenario Editor");
            writeBytes(getAbsAddress(xResScenarioEditorAddr), &xWantedRes, 4);
            writeBytes(getAbsAddress(yResScenarioEditorAddr), &yWantedRes, 4);
        }
    }

    showMessage("Post Resolution Settings");
}

void setGameSettings(gameSettings* tGameSet) {
    showMessage("Pre Game Settings");

    int* maxU_p = (int*)tracePointer(&maxUnitsPTR);
    writeBytes(maxU_p, &tGameSet->maxUnits, 4);

    showMessage("Post Game Settings");
}

void setCameraParams(cameraSettings* tSet) {
    showMessage("Pre Camera Params");

    if (!tSet->bCameraPatch) {
        showMessage("Skipped...");
        return;
    }

    showMessage(*(float*)getAbsAddress(MaxZHeightAddr));
    showMessage(*(float*)getAbsAddress(FOVAddr));
    showMessage(*(float*)getAbsAddress(FOGDistanceAddr));
    showMessage(*(float*)getAbsAddress(MaxPitchAddr));
    showMessage(*(int*)getAbsAddress(ZoomStyleAddr));
    
    writeBytes(getAbsAddress(MaxZHeightAddr), &tSet->fMaxZHeight, 4);
    //writeBytes(getAbsAddress(CurrZHeightAddr), &tSet->fMaxZHeight, 4);
    writeBytes(getAbsAddress(FOGDistanceAddr), &tSet->fFOGDistance, 4);
    writeBytes(getAbsAddress(FOVAddr), &tSet->fFOV, 4);
    writeBytes(getAbsAddress(ZoomStyleAddr), &tSet->zoomStyle, 4); 
    writeBytes(getAbsAddress(MaxPitchAddr), &tSet->fCameraPitch, 4);
    //writeBytes(getAbsAddress(CurrPitchAddr), &tSet->fCameraPitch, 4);

    showMessage(*(float*)getAbsAddress(MaxZHeightAddr));
    showMessage(*(float*)getAbsAddress(FOVAddr));
    showMessage(*(float*)getAbsAddress(FOGDistanceAddr));
    showMessage(*(float*)getAbsAddress(MaxPitchAddr));
    showMessage(*(int*)getAbsAddress(ZoomStyleAddr));

    showMessage("Post Camera Params");

}

bool isLoaded() {
    return 0 != *(int*)getAbsAddress(EEDataPtrAddr);
}

bool isPlaying() {
    float fMapValue = *(float*)getAbsAddress(playerMapIsLoaded);

    showMessage(fMapValue);

    return fMapValue > FLT_EPSILON;
}

bool isSupportedVersion() {
    char* currVerStr = (char*)getAbsAddress(versionStrStatic);

    showMessage("static version string:");
    showMessage(currVerStr);

    return strcmp(supportedEEC, currVerStr) == 0;
}

int MainEntry(threadSettings* tSettings) {
    FILE* f;

    if (tSettings->bDebugMode) {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        startupMessage();
        showMessage(tSettings->bWINE);
        showMessage(tSettings->bDebugMode);

        showMessage(tSettings->camera.fMaxZHeight);
        showMessage(tSettings->camera.fFOV);
        showMessage(tSettings->camera.fFOGDistance);
        showMessage(tSettings->camera.zoomStyle);
        showMessage(tSettings->camera.fCameraPitch);
        showMessage(tSettings->resolution.xResolution);
        showMessage(tSettings->resolution.yResolution);
        showMessage(tSettings->resolution.bForceScenarioEditor);
    }

    if (!isSupportedVersion()) {
        showMessage("this version of EE is not supported by Reborn.dll");
        return true;
    }

    setResolutions(&tSettings->resolution);

    if (tSettings->bWINE) // wine is running in game thread, so we need to throw it out (for now)
        return true;

    while (1) {
        Sleep(500);
        showMessage("Loop");

        if (isLoaded()) {
            // Patch and stop loop, we don't need anything else for the moment
            showMessage("EE is loaded...");
            setGameSettings(&tSettings->game);

            while (!isPlaying()) {
                showMessage("is not playing");
                Sleep(500);
            }

            setCameraParams(&tSettings->camera);
            break;
        }
        else
        {
            showMessage("EE is not loaded...");
        }
    }

    FreeConsole();
    return true;
}

DWORD WINAPI RebornDLLThread(LPVOID param) {
    std::cout << "3...\n";
    return MainEntry(reinterpret_cast<threadSettings*>(param));
}