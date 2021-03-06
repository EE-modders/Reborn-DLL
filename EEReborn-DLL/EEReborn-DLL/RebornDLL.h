#pragma once
#include <Windows.h>

#define RES_CUSTOM      0x3
#define RES_GAME        0x2
#define RES_WIN         0x1
#define RES_DISABLED    0x0

const int version_maj = 0;
const int version_min = 1;

const char supportedEEC[] = "2002.09.12.v2.00";


struct cameraSettings {
    bool bCameraPatch;              // enables camera patch
    int zoomStyle;                  // 0, 1, 2
    float fMaxZHeight;              // -0 to -99
    float fFOV;                     // 0.0 to 0.89 (crashes with and above 0.9)
    float fFOGDistance;             // 0.0 to 90.0 (crashes with and above 70.0)
    float fCameraPitch;             // -0 to -1 (under -1 inverts camera upside down)
};

struct resolutionSettings {
    int ResPatchType;               // enabled resolution patch (0 : Disabled | 1 : Windows | 2 : Game | 3 : Custom (Forced))
    bool bCustomResolution;         // enable custom resolutins specified
    int xResolution;                // 
    int yResolution;                // 

    // DANGER ZONE
    bool bForceScenarioEditor;      // force resolution of scenario editor (will look weird)
};

struct gameSettings {
    int maxUnits;
};

struct threadSettings {
    bool bWINE;                     // for internal use
    bool bDebugMode;                // enables debug mode
    struct resolutionSettings resolution;
    struct cameraSettings camera;
    struct gameSettings game;
};

/* functions */
DWORD WINAPI RebornDLLThread(LPVOID param);
int MainEntry(threadSettings* tSettings);
