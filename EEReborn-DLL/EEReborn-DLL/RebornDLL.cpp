#include "pch.h"
#include "RebornDLL.h"

/*###################################*/

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

void RebornDLL::getResolution(int& x, int& y, resolutionSettings* tSet) {

    switch (tSet->ResPatchType) {
    case RES_CUSTOM:
        x = tSet->xResolution;
        y = tSet->yResolution;
        break;
    case RES_GAME:
        x = *(int*)(getAbsAddress(gameMemory->xResSettingsAddr));
        y = *(int*)(getAbsAddress(gameMemory->yResSettingsAddr));
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

void resizeImage(const char *org, const char *dest, int x, int y)
{
    if (PathFileExistsA(org)) {
        showMessage("Resizing");
        showMessage(org);
        try {
            boost::gil::rgb8_image_t img;
            boost::gil::read_image(org, img, boost::gil::jpeg_tag{});

            boost::gil::rgb8_image_t wantedImgRes(x, y);
            boost::gil::resize_view(boost::gil::const_view(img), boost::gil::view(wantedImgRes), boost::gil::bilinear_sampler{});
            boost::gil::write_view(dest, boost::gil::const_view(wantedImgRes), boost::gil::jpeg_tag{});
            showMessage("Resizing OK");
        }
        catch (std::exception ex)
        {
            showMessage("Unnable to resize:");
            showMessage(ex.what());
        }
    }
    else
    {
        showMessage("Unnable to find:");
        showMessage(org);
    }
}

void RebornDLL::setLobbyImageResolution(int x, int y)
{
    resizeImage("./Data/WONLobby Resources/Images/main_menu_compilation_hq.jpg",
        "./Data/WONLobby Resources/Images/main_menu_compilation.jpg", x, y);
    if (gameType == GameType::EE) {
        resizeImage("./Data/WONLobby Resources/Images/bkg_german_hq.jpg",
            "./Data/WONLobby Resources/Images/bkg_german.jpg", x, y);
    }
    else if (gameType == GameType::AoC) {
        resizeImage("./Data/WONLobby Resources/Images/bkg_fleet_hq.jpg",
            "./Data/WONLobby Resources/Images/bkg_fleet.jpg", x, y);
    }
}

void RebornDLL::setResolutions(resolutionSettings* tSet) {
    showMessage("Pre Resolution Settings");

    if (tSet->ResPatchType == RES_DISABLED) {
        showMessage("Skipped...");
        return;
    }

    int xWantedRes, yWantedRes, xCurrentRes, yCurrentRes;
    bool bResMismatchX, bResMismatchY;

    getResolution(xWantedRes, yWantedRes, tSet);
    xCurrentRes = *(int*)(getAbsAddress(gameMemory->xResStartupAddr));
    yCurrentRes = *(int*)(getAbsAddress(gameMemory->yResStartupAddr));
    
    bResMismatchX = xWantedRes != xCurrentRes;
    bResMismatchY = yWantedRes != yCurrentRes;
    if (bResMismatchX || bResMismatchY) {
        std::cout << "Resolutions don't match" << std::endl;
        std::cout << "Current : " << xCurrentRes << "x" << yCurrentRes << std::endl;
        std::cout << "Wanted : " << xWantedRes << "x" << yWantedRes << std::endl;
        std::cout << "Set new resolution (type : " << tSet->ResPatchType << ")" << std::endl;

        writeBytes(getAbsAddress(gameMemory->xResStartupAddr), &xWantedRes, 4);
        writeBytes(getAbsAddress(gameMemory->yResStartupAddr), &yWantedRes, 4);
        writeBytes(getAbsAddress(gameMemory->yResBINKAddr), &yWantedRes, 4);

        writeBytes(getAbsAddress(gameMemory->xResStartupMainMenuAddr), &xWantedRes, 4);
        writeBytes(getAbsAddress(gameMemory->yResStartupMainMenuAddr), &yWantedRes, 4);

        writeBytes(getAbsAddress(gameMemory->xResMainMenuAddr), &xWantedRes, 4);
        writeBytes(getAbsAddress(gameMemory->yResMainMenuAddr), &yWantedRes, 4);

        setLobbyImageResolution(xWantedRes, yWantedRes);

        if (tSet->bForceScenarioEditor) {
            showMessage("Forced Scenario Editor");
            writeBytes(getAbsAddress(gameMemory->xResScenarioEditorAddr), &xWantedRes, 4);
            writeBytes(getAbsAddress(gameMemory->yResScenarioEditorAddr), &yWantedRes, 4);
        }
    }

    showMessage("Post Resolution Settings");
}

void RebornDLL::setGameSettings(gameSettings* tGameSet) {
    showMessage("Pre Game Settings");

    if (tGameSet->maxUnits != 0) {
        showMessage(&tGameSet->maxUnits);
        int* maxU_p = (int*)tracePointer(&gameMemory->maxUnitsPTR);
        writeBytes(maxU_p, &tGameSet->maxUnits, 4);
    }

    showMessage("Post Game Settings");
}

void RebornDLL::setCameraParams(cameraSettings* tSet) {
    showMessage("Pre Camera Params");

    if (!tSet->bCameraPatch) {
        showMessage("Skipped...");
        return;
    }

    showMessage(*(float*)getAbsAddress(gameMemory->MaxZHeightAddr));
    showMessage(*(float*)getAbsAddress(gameMemory->FOVAddr));
    showMessage(*(float*)getAbsAddress(gameMemory->FOGDistanceAddr));
    showMessage(*(float*)getAbsAddress(gameMemory->MaxPitchAddr));
    showMessage(*(int*)getAbsAddress(gameMemory->ZoomStyleAddr));
    
    writeBytes(getAbsAddress(gameMemory->MaxZHeightAddr), &tSet->fMaxZHeight, 4);
    //writeBytes(getAbsAddress(CurrZHeightAddr), &tSet->fMaxZHeight, 4);
    writeBytes(getAbsAddress(gameMemory->FOGDistanceAddr), &tSet->fFOGDistance, 4);
    writeBytes(getAbsAddress(gameMemory->FOVAddr), &tSet->fFOV, 4);
    writeBytes(getAbsAddress(gameMemory->ZoomStyleAddr), &tSet->zoomStyle, 4);
    writeBytes(getAbsAddress(gameMemory->MaxPitchAddr), &tSet->fCameraPitch, 4);
    //writeBytes(getAbsAddress(CurrPitchAddr), &tSet->fCameraPitch, 4);

    showMessage(*(float*)getAbsAddress(gameMemory->MaxZHeightAddr));
    showMessage(*(float*)getAbsAddress(gameMemory->FOVAddr));
    showMessage(*(float*)getAbsAddress(gameMemory->FOGDistanceAddr));
    showMessage(*(float*)getAbsAddress(gameMemory->MaxPitchAddr));
    showMessage(*(int*)getAbsAddress(gameMemory->ZoomStyleAddr));

    showMessage("Post Camera Params");

}

bool RebornDLL::isLoaded() {
    return 0 != *(int*)getAbsAddress(gameMemory->EEDataPtrAddr);
}

bool RebornDLL::isPlaying() {
    return 0 != *(int*)getAbsAddress(gameMemory->EEMapPtrAddr);
}

bool RebornDLL::isSupportedVersion() {
    char* currVerStr = (char*)getAbsAddress(gameMemory->versionStrStatic);

    showMessage("static version string:");
    showMessage(currVerStr);

    return strcmp(supportedEEC, currVerStr) == 0;
}

RebornDLL::RebornDLL(threadSettings* tSettings)
{
    this->tSettings = tSettings;
}

int RebornDLL::MainEntry() {
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


    if (PathFileExists(L"./Empire Earth.exe"))
        gameType = GameType::EE;
    else if (PathFileExists(L"./EE-AoC.exe"))
        gameType = GameType::AoC;
    else
        gameType = GameType::NA;

    gameMemory = new GameMemory(gameType);

    if (gameType == GameType::EE) {
        if (!isSupportedVersion()) {
            showMessage("This version of EE is not supported by Reborn.dll");
            return true;
        }
    }
    else if (gameType == GameType::AoC) {
        showMessage("AoC isn't supported yet !");
        return true;
    }
    else {
        showMessage("Unnable to detect EE & AoC ! Check the name of the game executable !");
    }


    setResolutions(&tSettings->resolution);

    if (tSettings->bWINE) {
        showMessage("WINE detected!");
    }

    while (!isLoaded()) {
        showMessage("EE is not loaded");
        Sleep(500);
    }

    showMessage("EE is loaded");
    setGameSettings(&tSettings->game);

    while (1) {
        Sleep(250);
        
        if (isPlaying() && !bWasPlaying) {
            showMessage("started playing");

            setCameraParams(&tSettings->camera);
            bWasPlaying = true;
        }

        if (!isPlaying() && bWasPlaying) {
            showMessage("stopped playing");
            bWasPlaying = false;
        }
    }

    FreeConsole();
    return true;
}

/* Disabled Wine Fix
DWORD WINAPI RebornDLL::RebornDLLThread(LPVOID param) {
    std::cout << "Running WINE path...\n";
    RebornDLL *reborn = new RebornDLL(reinterpret_cast<threadSettings*>(param));
    return reborn->MainEntry();
}*/