#pragma once
#include <Windows.h>

enum class GameType
{
    NA = -1, EE = 0, AoC = 1
};

struct memoryPTR {
    DWORD base_address;
    int total_offsets;
    int offsets[];
};


void protectedCpy(void* dest, void* src, int n);
bool readBytes(void* read_addr, void* read_buffer, int len);
void writeBytes(void* dest_addr, void* patch, int len);
HMODULE getBaseAddress();
DWORD* getAbsAddress(DWORD appl_addr);
DWORD* tracePointer(memoryPTR* patch);

class GameMemory
{

public:
    GameMemory(GameType type);

    DWORD MaxZHeightAddr = 0x0;             // float
    DWORD CurrZHeightAddr = 0x0;            // float
    DWORD LastZHeightAddr = 0x0;            // float
    DWORD ZoomStateAddr = 0x0;              // 0.0: fully zoomed in = 0x0; 1.0: fully zoomed out | float
    DWORD ZoomStyleAddr = 0x0;              // 0, 1, 2 | int
    DWORD FOVAddr = 0x0;                    // float
    DWORD FOGDistanceAddr = 0x0;            // float
    DWORD MaxPitchAddr = 0x0;               // 0.0: 0d pitch = 0x0; -1.0: 90d pitch | float
    DWORD CurrPitchAddr = 0x0;              // float

    DWORD resSwitchCheckAddr = 0x0;         // 2 bytes containing JE 15

    DWORD versionStrPtrAddr = 0x0;          // version string pointer
    DWORD versionStrStatic = 0x0;           // static version string

    // all following values are int
    DWORD xResSettingsAddr = 0x0;           // xRes set in the ingame settings
    DWORD yResSettingsAddr = 0x0;
    DWORD bitSettingsAddr = 0x0;

    DWORD xResStartupAddr = 0x0;            // initial x-resolution
    DWORD yResStartupAddr = 0x0;            // initial y-resolution
    DWORD yResBINKAddr = 0x0;               // y-axis resolution limit for BINK videos

    DWORD xResStartupMainMenuAddr = 0x0;    // X main menu resolution set when starting
    DWORD yResStartupMainMenuAddr = 0x0;    // Y main menu resolution set when starting

    DWORD xResMainMenuAddr = 0x0;           // X main menu resolution when not starting up
    DWORD yResMainMenuAddr = 0x0;           // Y main menu resolution when not starting up

    DWORD xResScenarioEditorAddr = 0x0;     // X scenario editor resoluton
    DWORD yResScenarioEditorAddr = 0x0;     // Y scenario editor resoluton
    // ---

    // EE Objects

    DWORD EEDataPtrAddr = 0x0;              // this will be non 00 when EEData is loaded
    DWORD EEMapPtrAddr = 0x0;               // this will be non 00 when EEMap is loaded

    // ---

    memoryPTR maxUnitsPTR = {
        0x0,
        0,
        { 0x0 }
    };

};