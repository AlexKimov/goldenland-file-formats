#include "IATHook.h"
#include "Logger.h"

namespace IATHook {

    bool Patch(const char* dllName, const char* funcName, void* hookFunc, void** origFunc, HMODULE hModule) {
        if (!hModule) {
            hModule = GetModuleHandleA(NULL);
        }
        if (!hModule) {
            LOG_BASIC("IATHook::Patch failed - cannot get module handle");
            return false;
        }

        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
        if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

        PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);
        if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) return false;

        DWORD importRVA = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (importRVA == 0) return false;

        PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hModule + importRVA);

        while (pImportDesc->Name != 0) {
            const char* impDllName = (const char*)((BYTE*)hModule + pImportDesc->Name);

            if (_stricmp(impDllName, dllName) == 0) {
                PIMAGE_THUNK_DATA pOrigThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + pImportDesc->OriginalFirstThunk);
                PIMAGE_THUNK_DATA pFirstThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + pImportDesc->FirstThunk);

                while (pOrigThunk->u1.AddressOfData != 0) {
                    if (!(pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                        PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hModule + pOrigThunk->u1.AddressOfData);

                        if (strcmp((const char*)pImportByName->Name, funcName) == 0) {
                            DWORD oldProtect;
                            VirtualProtect(&pFirstThunk->u1.Function, sizeof(DWORD_PTR), PAGE_EXECUTE_READWRITE, &oldProtect);

                            *origFunc = (void*)pFirstThunk->u1.Function;
                            pFirstThunk->u1.Function = (DWORD_PTR)hookFunc;

                            VirtualProtect(&pFirstThunk->u1.Function, sizeof(DWORD_PTR), oldProtect, &oldProtect);

                            LOG_BASIC("IATHook: Patched %s!%s -> %p (orig: %p)", dllName, funcName, hookFunc, *origFunc);
                            return true;
                        }
                    }
                    pOrigThunk++;
                    pFirstThunk++;
                }
            }
            pImportDesc++;
        }

        LOG_BASIC("IATHook: Function %s!%s not found in IAT", dllName, funcName);
        return false;
    }

} // namespace IATHook