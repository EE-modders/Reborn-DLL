#include "pch.h"
#include "GameMemory.h"

GameMemory::GameMemory(GameType type)
{
    if (type == GameType::EE) {

        // CAMERA
        MaxZHeightAddr = 0x42C700;
        CurrZHeightAddr = 0x518d40;
        LastZHeightAddr = 0x518D08;
        ZoomStateAddr = 0x518DB0;
        ZoomStyleAddr = 0x518dc4;
        FOVAddr = 0x51328c;
        FOGDistanceAddr = 0x42c704;
        MaxPitchAddr = 0x513284;
        CurrPitchAddr = 0x5183c8;

        // VERSION
        versionStrPtrAddr = 0x1D16FB;
        versionStrStatic = 0x4A9030;

        // RESOLUTION
        resSwitchCheckAddr = 0x25FAC2;

        xResSettingsAddr = 0x5193FC;
        yResSettingsAddr = 0x5193F8;
        bitSettingsAddr = 0x5193F4;

        xResStartupAddr = 0x137523;
        yResStartupAddr = 0x137528;
        yResBINKAddr = 0x13753C;

        xResStartupMainMenuAddr = 0x138B3D;
        yResStartupMainMenuAddr = 0x138B38;

        xResMainMenuAddr = 0x25FACE;
        yResMainMenuAddr = 0x25FAC9;

        xResScenarioEditorAddr = 0x2601EB;
        yResScenarioEditorAddr = 0x2601E6;

        // Objects

        EEDataPtrAddr = 0x517BB8;
        EEMapPtrAddr = 0x00518378 + 0x44;

        // GAME

        maxUnitsPTR = {
            EEDataPtrAddr,
            1,
            { 0x2EC }
        };
    }
    else if (type == GameType::AoC) {
        // TODO
    }
}

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
