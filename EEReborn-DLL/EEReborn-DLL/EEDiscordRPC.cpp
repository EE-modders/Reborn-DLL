#include "pch.h"
#include "EEDiscordRPC.h"

#include <achievement_manager.cpp>
#include <activity_manager.cpp>
#include <application_manager.cpp>
#include <core.cpp>
#include <image_manager.cpp>
#include <lobby_manager.cpp>
#include <network_manager.cpp>
#include <overlay_manager.cpp>
#include <relationship_manager.cpp>
#include <storage_manager.cpp>
#include <store_manager.cpp>
#include <types.cpp>
#include <user_manager.cpp>
#include <voice_manager.cpp>

DiscordState state{};
discord::Core* core{};

// From https://stackoverflow.com/a/14751302
bool IsProcessRunning(const wchar_t* processName)
{
    bool exists = false;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry)) {
        while (Process32Next(snapshot, &entry)) {
            if (!wcsicmp(entry.szExeFile, processName)) {
                exists = true;
                break;
            }
        }
    }
    CloseHandle(snapshot);
    return exists;
}

void showMessage_(void* val) {
    std::cout << "DEBUG: " << val << std::endl;
}
void showMessage_(std::string val) {
    std::cout << "DEBUG: " << val << std::endl;
}

void EEDiscordRPC::updateStatus(std::string title, std::string subtext)
{
    state.core.get()->RunCallbacks();

    state.core->UserManager().OnCurrentUserUpdate.Connect([&state]() {
        state.core->UserManager().GetCurrentUser(&state.currentUser);

        showMessage_("Current user updated: ");
        showMessage_(state.currentUser.GetUsername());
        showMessage_("#");
        showMessage_(state.currentUser.GetDiscriminator());
    });
}

EEDiscordRPC::EEDiscordRPC(GameType gameType)
{
    discord::Core* core = nullptr;
    discord::Activity activity{};

    if (!init) {

        // Not really usefull, discord sdk probably already do those checks
        if (!IsWindows7OrGreater()) {
            showMessage_("RPC Init: Discord require >= Windows 7 !");
            return;
        }
        if (!IsProcessRunning(L"Discord.exe") || !IsProcessRunning(L"discord.exe")) {
            showMessage_("RPC Init: Discord isn't open !");
            return;
        }
        // End stupidity


        if (!PathFileExists(L"./discord_game_sdk.dll")) {
            showMessage_("RPC Init: Unnable to find discord_game_sdk.dll !");
            return;
        }

        discord::ClientId id = 0;

        if (gameType == GameType::EE)
            id = 782679873856077914;
        else if (gameType == GameType::AoC)
            id = 783353615670706217;
        else {
            showMessage_("RPC Init: Invalid GameType !");
            return;
        }

        auto result = discord::Core::Create(id, DiscordCreateFlags_Default, &core);
        if (result != discord::Result::Ok) {
            showMessage_("RPC Init: Failed to instantiate discord core !"
                " (err " + std::to_string(static_cast<int>(result)) + ")");
            return;
        }

        state.core.reset(core);

        if (!state.core) {
            showMessage_("RPC Init: Failed to instantiate !");
            return;
        }

        state.core->SetLogHook(
            discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
                std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
            });

        init = true;
        showMessage_("RPC Init: OK");
    }
}


    //    Discord_activity.SetType(discord::ActivityType::Playing);
    //    //activity.SetType(discord::ActivityType::Watching);
    //    Discord_activity.GetAssets().SetLargeImage(SetLargeImageDefault.c_str());
    //    Discord_activity.GetAssets().SetLargeText(SetLargeTextDefault.c_str());
    //    Discord_activity.GetAssets().SetSmallImage(SetSmallImageDefault.c_str());
    //    Discord_activity.GetAssets().SetSmallText(SetSmallTextDefault.c_str());
    //    Discord_activity.SetDetails(SetDetailsDefault.c_str());
    //    Discord_activity.SetState(SetStateDefault.c_str());
    //    Discord_activity.GetTimestamps().SetStart(SetStartDefault);
    //    Discord_activity.GetTimestamps().SetEnd(SetEndDefault);