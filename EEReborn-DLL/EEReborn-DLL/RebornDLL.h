#pragma once
#include <Windows.h>

const int version_maj = 1;
const int version_min = 1;

struct memoryPTR {
    DWORD base_address;
    int total_offsets;
    int offsets[];
};

struct cameraSettings {
    bool bCameraPatch;              // enables camera patch
    int zoomStyle;                  // 1, 2 and 3
    float fMaxZHeight;              // -0 to -99
    float fFOV;                     // 0.0 to 0.89 (crashes with and above 0.9)
    float fFOGDistance;             // 0.0 to 90.0 (crashes with and above 70.0)
    float fCameraPitch;             // -0 to -1 (under -1 inverts camera upside down)
};

struct resolutionSettings {
    bool bResPatch;                 // enabled resolution patch
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
