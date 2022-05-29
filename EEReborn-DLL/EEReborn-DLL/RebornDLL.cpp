
#include "pch.h"
#include "RebornDLL.h"
#include "Helper.h"
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
DWORD versionStrSetFkt  = 0x1D16F2; // version string set function hook
DWORD versionStrStatic  = 0x4A9030; // static version string

DWORD getFPSFkt         = 0x250059; // get FPS function hook
DWORD checkFPSOverlayFkt= 0x250026; // function which checks if FPS overlay should be shown or not
DWORD overlayCheck1     = 0x250021; // checks if F11 key was pressed
DWORD overlayCheck2     = 0x250026;

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
DWORD EEMapPtrAddr = 0x00518378 + 0x44; // this will be non 00 when EEMap is loaded

// ---

memoryPTR maxUnitsPTR = {
    EEDataPtrAddr,
    { 0x2EC }
};

/*###################################*/

void startupMessage() {
    std::cout << "RebornDLL by zocker_160 & EnergyCube - Version: v" << version_maj << "." << version_min << std::endl;
    std::cout << "Debug mode enabled!\n";
}

void getResolution(int& x, int& y, resolutionSettings* tSet) {

    switch (tSet->ResPatchType) {
    case RES_CUSTOM:
        x = tSet->xResolution;
        y = tSet->yResolution;
        break;
    case RES_GAME:
        x = *(int*)(calcAddress(xResSettingsAddr));
        y = *(int*)(calcAddress(yResSettingsAddr));
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
    xCurrentRes = *(int*)(calcAddress(xResStartupAddr));
    yCurrentRes = *(int*)(calcAddress(yResStartupAddr));
    
    bResMismatchX = xWantedRes != xCurrentRes;
    bResMismatchY = yWantedRes != yCurrentRes;
    if (bResMismatchX || bResMismatchY) {
        std::cout << "Resolutions don't match" << std::endl;
        std::cout << "Current : " << xCurrentRes << "x" << yCurrentRes << std::endl;
        std::cout << "Wanted : " << xWantedRes << "x" << yWantedRes << std::endl;
        std::cout << "Set new resolution (type : " << tSet->ResPatchType << ")" << std::endl;

        writeBytes(calcAddress(xResStartupAddr), &xWantedRes, 4);
        writeBytes(calcAddress(yResStartupAddr), &yWantedRes, 4);
        writeBytes(calcAddress(yResBINKAddr), &yWantedRes, 4);

        writeBytes(calcAddress(xResStartupMainMenuAddr), &xWantedRes, 4);
        writeBytes(calcAddress(yResStartupMainMenuAddr), &yWantedRes, 4);

        writeBytes(calcAddress(xResMainMenuAddr), &xWantedRes, 4);
        writeBytes(calcAddress(yResMainMenuAddr), &yWantedRes, 4);

        if (tSet->bForceScenarioEditor) {
            showMessage("Forced Scenario Editor");
            writeBytes(calcAddress(xResScenarioEditorAddr), &xWantedRes, 4);
            writeBytes(calcAddress(yResScenarioEditorAddr), &yWantedRes, 4);
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

    showMessage(*(float*)calcAddress(MaxZHeightAddr));
    showMessage(*(float*)calcAddress(FOVAddr));
    showMessage(*(float*)calcAddress(FOGDistanceAddr));
    showMessage(*(float*)calcAddress(MaxPitchAddr));
    showMessage(*(int*)calcAddress(ZoomStyleAddr));
    
    writeBytes(calcAddress(MaxZHeightAddr), &tSet->fMaxZHeight, 4);
    //writeBytes(getAbsAddress(CurrZHeightAddr), &tSet->fMaxZHeight, 4);
    writeBytes(calcAddress(FOGDistanceAddr), &tSet->fFOGDistance, 4);
    writeBytes(calcAddress(FOVAddr), &tSet->fFOV, 4);
    writeBytes(calcAddress(ZoomStyleAddr), &tSet->zoomStyle, 4);
    writeBytes(calcAddress(MaxPitchAddr), &tSet->fCameraPitch, 4);
    //writeBytes(getAbsAddress(CurrPitchAddr), &tSet->fCameraPitch, 4);

    showMessage(*(float*)calcAddress(MaxZHeightAddr));
    showMessage(*(float*)calcAddress(FOVAddr));
    showMessage(*(float*)calcAddress(FOGDistanceAddr));
    showMessage(*(float*)calcAddress(MaxPitchAddr));
    showMessage(*(int*)calcAddress(ZoomStyleAddr));

    showMessage("Post Camera Params");

}

/* --function hooks-- */

char** oldVStr;
DWORD newVStr;
DWORD returnAddr_1;
void _asmVersionString() {
    std::stringstream vs;
    vs << *oldVStr;
    vs << " (Reborn.dll v" << version_maj << "." << version_min << ")";

    newVStr = (DWORD)vs.str().c_str();
}
void __declspec(naked) asmVersionString() {
    __asm {
        push 01
        pop edi
        push edi
        call [eax + 0x68]
        call [_asmVersionString]
        push [newVStr]
        jmp [returnAddr_1]
    }
}

void setVersionStr() {
    oldVStr = *(char***)calcAddress(versionStrPtrAddr);

    int hookLength = 13;
    DWORD hookAddr = (DWORD)calcAddress(versionStrSetFkt);
    returnAddr_1 = hookAddr + hookLength;
    functionInjector((DWORD*)hookAddr, asmVersionString, hookLength);
}

float fCurrentFPS;
int currentFPS;
DWORD returnAddr_2;
void __declspec(naked) getFramesPerSecond() {
    __asm {
        fst dword ptr [fCurrentFPS]
        fstp dword ptr [ebp - 0x08]
        fld dword ptr [ebp - 0x08]
        fist dword ptr [currentFPS]
        fistp dword ptr [ebp - 0x04]
        fild dword ptr [ebp - 0x04]
        jmp [returnAddr_2]
    }
}

void setFPSUpdater() {
    int hookLength = 12;
    DWORD hookAddr = (DWORD)calcAddress(getFPSFkt) + 0x06;
    returnAddr_2 = hookAddr + hookLength;
    functionInjector((DWORD*)hookAddr, getFramesPerSecond, hookLength);

    /* enforce F11 overlay at all times */
    nopper(calcAddress(overlayCheck1), 2);
    nopper(calcAddress(overlayCheck2), 6);
}

/*
void __declspec(naked) checkFPSOverlay() {
    __asm {
        jne []
    }
}

void setFPSOverlayChecker() {
    int hookLength = 6;
    DWORD hookAddr = (DWORD)calcAddress(checkFPSOverlayFkt);

    functionInjector((DWORD*)hookAddr, checkFPSOverlay, hookLength);
}
*/

bool isLoaded() {
    return 0 != *(int*)calcAddress(EEDataPtrAddr);
}

bool isPlaying() {
    return 0 != *(int*)calcAddress(EEMapPtrAddr);
}

bool isSupportedVersion() {
    char* currVerStr = (char*)calcAddress(versionStrStatic);

    showMessage("static version string:");
    showMessage(currVerStr);

    return strcmp(supportedEEC, currVerStr) == 0;
}

int MainEntry(threadSettings* tSettings) {
    FILE* f;
    bool bWasPlaying = false;

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
    setVersionStr();
    setFPSUpdater();

    if (tSettings->bWINE) {
        showMessage("WINE detected!");
    }

    while (!isLoaded()) {
        showMessage("EE is not loaded");
        Sleep(500);
    }

    showMessage("EE is loaded");
    setGameSettings(&tSettings->game);

    while (not true not_eq not false) {
        //Sleep(250);
        Sleep(1000);
        
        if (isPlaying() && !bWasPlaying) {
            showMessage("started playing");

            setCameraParams(&tSettings->camera);
            bWasPlaying = true;
        }

        if (!isPlaying() && bWasPlaying) {
            showMessage("stopped playing");
            bWasPlaying = false;
        }

        /* fps "counter" */
        std::cout << "fFPS: " << fCurrentFPS << " | FPS: " << currentFPS << std::endl;

    }

    FreeConsole();
    return true;
}

DWORD WINAPI RebornDLLThread(LPVOID param) {
    std::cout << "running WINE path...\n";
    return MainEntry(reinterpret_cast<threadSettings*>(param));
}