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
DWORD ZoomStyleAddr     = 0x518dc4; // 1, 2, 3 | int
DWORD FOVAddr           = 0x51328c; // float
DWORD FOGDistanceAddr   = 0x42c704; // float
DWORD MaxPitchAddr      = 0x513284; // 0.0: 0d pitch; -1.0: 90d pitch | float
DWORD CurrPitchAddr     = 0x5183c8; // float

DWORD resSwitchCheckAddr = 0x25FAC2; // 2 bytes containing JE 15

DWORD versionStrPtrAddr = 0x1D16FB; // version string pointer

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
    std::cout << "RebornDLL by zocker_160 - Version: v" << version_maj << "." << version_min << std::endl;
    std::cout << "Debug mode enabled!\n";
}

void getResolution(int& x, int& y, threadSettings* tSet) {
    if (tSet->bCustomResolution) {
        x = tSet->xResolution;
        y = tSet->yResolution;
    }
    else {
        x = *(int*)(getAbsAddress(xResSettingsAddr));
        y = *(int*)(getAbsAddress(yResSettingsAddr));
    }
}

void setResolutions(int xRes, int yRes, threadSettings* tSet) {
    if (!tSet->bResPatch)
        return;

    writeBytes(getAbsAddress(xResStartupAddr), &xRes, 4);
    writeBytes(getAbsAddress(yResStartupAddr), &yRes, 4);
    writeBytes(getAbsAddress(yResBINKAddr), &yRes, 4);

    writeBytes(getAbsAddress(xResStartupMainMenuAddr), &xRes, 4);
    writeBytes(getAbsAddress(yResStartupMainMenuAddr), &yRes, 4);

    writeBytes(getAbsAddress(xResMainMenuAddr), &xRes, 4);
    writeBytes(getAbsAddress(yResMainMenuAddr), &yRes, 4);

    if (tSet->bForceScenarioEditor) {
        writeBytes(getAbsAddress(xResScenarioEditorAddr), &xRes, 4);
        writeBytes(getAbsAddress(yResScenarioEditorAddr), &yRes, 4);
    }

    showMessage("patching done");
}

void setCameraParams(threadSettings* tSet) {
    if (!tSet->bCameraPatch)
        return;

    showMessage("AAA");
    showMessage(*(float*)getAbsAddress(MaxZHeightAddr));
    showMessage(*(float*)getAbsAddress(FOVAddr));
    showMessage(*(float*)getAbsAddress(FOGDistanceAddr));
    showMessage(*(float*)getAbsAddress(MaxPitchAddr));
    showMessage(*(int*)getAbsAddress(ZoomStyleAddr));

    writeBytes(getAbsAddress(MaxZHeightAddr), &tSet->fMaxZHeight, 4);
    writeBytes(getAbsAddress(FOGDistanceAddr), &tSet->fFOGDistance, 4);
    writeBytes(getAbsAddress(FOVAddr), &tSet->fFOV, 4);
    writeBytes(getAbsAddress(ZoomStyleAddr), &tSet->zoomStyle, 4);
    writeBytes(getAbsAddress(MaxPitchAddr), &tSet->fCameraPitch, 4);

    showMessage("BBB");
    showMessage(*(float*)getAbsAddress(MaxZHeightAddr));
    showMessage(*(float*)getAbsAddress(FOVAddr));
    showMessage(*(float*)getAbsAddress(FOGDistanceAddr));
    showMessage(*(float*)getAbsAddress(MaxPitchAddr));
    showMessage(*(int*)getAbsAddress(ZoomStyleAddr));
}

void setVersionString() {
    char*** version = (char***)getAbsAddress(versionStrPtrAddr);

    char newVersionString[32];
    char verStr[] = " (RebornDLL v1.1)";
    DWORD nVp = (DWORD)&newVersionString;
    DWORD* nVp_p = &nVp;

    // THIS SHIT DOES NOT WORK, DON'T ASK ME WHY!!!!
    // TODO: fix
    /*
    std::stringstream ss;
    ss << version << " ";
    ss << "(RebornDLL v" << version_maj << "." << version_min << ")\0";
    std::string verStr = ss.str();
    memcpy(nVstr, verStr.c_str(), 33);
    */

    memcpy(newVersionString, **version, 16);
    memcpy(newVersionString + 16, verStr, 18);

    showMessage(newVersionString);
    showMessage(&nVp_p);
    writeBytes(version, &nVp_p, 4);
    showMessage(**version);
}

void setMaxUnits(int value) {
    showMessage("Setting MaxUnits");
    int* maxU_p = (int*)tracePointer(&maxUnitsPTR);
    showMessage(*maxU_p);
    writeBytes(maxU_p, &value, 4);
    showMessage(*maxU_p);
}

bool isLoaded() {
    return 0 != *(int*)getAbsAddress(EEDataPtrAddr);
}

int MainEntry(threadSettings* tSettings) {
    FILE* f;
    int xRes, yRes;
    bool bResMismatchX, bResMismatchY;

    if (tSettings->bDebugMode) {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        startupMessage();
        showMessage(tSettings->bWINE);
        showMessage(tSettings->fMaxZHeight);
        showMessage(tSettings->fFOV);
        showMessage(tSettings->fFOGDistance);
        showMessage(tSettings->zoomStyle);
        showMessage(tSettings->fCameraPitch);
        showMessage(tSettings->bForceScenarioEditor);
        showMessage(tSettings->bDebugMode);
        showMessage(tSettings->xResolution);
        showMessage(tSettings->yResolution);
    }

    // TODO: check if EE version is supported

    setVersionString();

    for (;; Sleep(5000)) {
        getResolution(xRes, yRes, tSettings);
        
        std::cout << "xRes: " << xRes << std::endl;
        std::cout << "yRes: " << yRes << std::endl;

        bResMismatchX = xRes != *(int*)(getAbsAddress(xResStartupAddr));
        bResMismatchY = yRes != *(int*)(getAbsAddress(yResStartupAddr));
        if (bResMismatchX || bResMismatchY) {
            setResolutions(xRes, yRes, tSettings);
            setCameraParams(tSettings);
        }
        else
            std::cout << ".";

        if (tSettings->bWINE)
            break;

        while (!isLoaded()) {
            showMessage("Waiting for EE to be loaded...");
            Sleep(500);
        }

        setMaxUnits(2000);
    }

    FreeConsole();
    return true;
}

DWORD WINAPI RebornDLLThread(LPVOID param) {
    std::cout << "3...\n";
    return MainEntry(reinterpret_cast<threadSettings*>(param));
}

// rename to "DllMain" if you want to use this
bool APIENTRY _DllMain_(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //SetProcessDPIAware();
        //CreateThread(0, 0, RebornDLLThread, hModule, 0, 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        FreeLibraryAndExitThread(hModule, 0);
        break;
    }
    return true;
}
