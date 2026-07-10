#pragma once
#include <windows.h>

namespace InlineHook {
    struct HookInfo {
        void* targetAddr;
        void* trampoline;
        size_t patchSize;
        BYTE originalBytes[32]; 
    };

    HookInfo* Install(void* targetAddr, void* hookAddr, size_t patchSize = 5);

    void Remove(HookInfo* info);
}