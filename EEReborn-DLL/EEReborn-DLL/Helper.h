/*
 * C++ helper for memory hacking by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#pragma once
#include <Windows.h>
#include <vector>

struct memoryPTR {
    DWORD base_address;
    std::vector<int> offsets;
};

void protectedRead(void* dest, void* src, int n);
bool readBytes(void* read_addr, void* read_buffer, int len);
void writeBytes(void* dest_addr, void* patch, int len);

HMODULE getBaseAddress();
HMODULE getMatchmakingAddress();

DWORD* calcAddress(DWORD appl_addr);
DWORD* tracePointer(memoryPTR* patch);

bool functionInjector(void* toHook, void* function, int len);

void showMessage(float val);
void showMessage(int val);
void showMessage(short val);
void showMessage(char* val);
void showMessage(LPCSTR val);

void GetDesktopResolution(int& horizontal, int& vertical);
void GetDesktopResolution2(int& hor, int& vert);
float calcAspectRatio(int horizontal, int vertical);

bool IsKeyPressed(int vKey);
