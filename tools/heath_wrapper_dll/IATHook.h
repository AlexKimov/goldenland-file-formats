#pragma once
#include <windows.h>

namespace IATHook {
    bool Patch(const char* dllName, const char* funcName, void* hookFunc, void** origFunc, HMODULE hModule = NULL);
}