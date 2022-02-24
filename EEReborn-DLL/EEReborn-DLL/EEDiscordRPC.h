#pragma once

#include "GameMemory.h"
#include <string>

#include <VersionHelpers.h>
#include <Shlwapi.h>

#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>

#include <array>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include <discord.h>

struct DiscordState {
	discord::User currentUser;

	std::unique_ptr<discord::Core> core;
};

class EEDiscordRPC
{
public:
	volatile bool interrupted { false };

	bool init = false;

	EEDiscordRPC(GameType gameType);
	void updateStatus(std::string title, std::string subtext);

private:
};