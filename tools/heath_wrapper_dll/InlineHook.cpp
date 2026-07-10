#include "InlineHook.h"

namespace InlineHook {

    HookInfo* Install(void* targetAddr, void* hookAddr, size_t patchSize) {
        if (patchSize < 5) return nullptr; // JMP rel32 занимает ровно 5 байт

        HookInfo* info = new HookInfo();
        info->targetAddr = targetAddr;
        info->patchSize = patchSize;

        // 1. Сохраняем оригинальные байты
        memcpy(info->originalBytes, targetAddr, patchSize);

        // 2. Выделяем память для трамплина (оригинальные байты + JMP обратно)
        size_t trampolineSize = patchSize + 5;
        info->trampoline = VirtualAlloc(NULL, trampolineSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!info->trampoline) {
            delete info;
            return nullptr;
        }

        // 3. Копируем оригинальные инструкции в трамплин
        memcpy(info->trampoline, info->originalBytes, patchSize);

        // 4. Добавляем JMP обратно в оригинальную функцию (сразу после пропатченных байт)
        BYTE* trampolineJmp = (BYTE*)info->trampoline + patchSize;
        trampolineJmp[0] = 0xE9; // Opcode для JMP rel32
        DWORD jmpBackRel = (DWORD)((BYTE*)targetAddr + patchSize) - (DWORD)(trampolineJmp + 5);
        *(DWORD*)(trampolineJmp + 1) = jmpBackRel;

        // 5. Снимаем защиту памяти с целевой функции
        DWORD oldProtect;
        if (!VirtualProtect(targetAddr, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            VirtualFree(info->trampoline, 0, MEM_RELEASE);
            delete info;
            return nullptr;
        }

        // 6. Патчим целевую функцию: ставим JMP на наш hookAddr
        BYTE* targetBytes = (BYTE*)targetAddr;
        targetBytes[0] = 0xE9; // Opcode для JMP rel32
        DWORD hookRel = (DWORD)hookAddr - (DWORD)(targetBytes + 5);
        *(DWORD*)(targetBytes + 1) = hookRel;

        // 7. Если patchSize > 5, забиваем оставшиеся байты NOP-ами (0x90)
        for (size_t i = 5; i < patchSize; i++) {
            targetBytes[i] = 0x90;
        }

        // 8. Возвращаем защиту памяти и сбрасываем кэш инструкций процессора
        VirtualProtect(targetAddr, patchSize, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), targetAddr, patchSize);

        return info;
    }

    void Remove(HookInfo* info) {
        if (!info) return;

        DWORD oldProtect;
        if (VirtualProtect(info->targetAddr, info->patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            // Восстанавливаем оригинальные байты
            memcpy(info->targetAddr, info->originalBytes, info->patchSize);
            VirtualProtect(info->targetAddr, info->patchSize, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), info->targetAddr, info->patchSize);
        }

        if (info->trampoline) {
            VirtualFree(info->trampoline, 0, MEM_RELEASE);
        }
        delete info;
    }
}