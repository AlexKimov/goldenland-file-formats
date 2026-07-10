#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <initguid.h>
#include <ddraw.h>

#include "DirectDrawWrapper.h"
#include "WindowManager.h"
#include "Settings.h"
#include "Logger.h"
#include "Dumper.h"

HMODULE g_hDllModule = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        Settings::GetInstance().Load(".\\wrapper.ini");
        Logger::GetInstance().Init(".\\d3d9_wrapper.log");
        Dumper::GetInstance().Init();

        if (!WindowManager::GetInstance().Initialize()) {
            LOG_BASIC("Failed to initialize WindowManager");
            return FALSE;
        }

        g_hDllModule = hModule;

        LOG_BASIC("=== D3D9 Wrapper Loaded Successfully ===");
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        WindowManager::GetInstance().Shutdown();

        LOG_BASIC("=== D3D9 Wrapper Unloaded ===");
        Logger::GetInstance().Close();
    }
    return TRUE;
}


extern "C" {

    HRESULT WINAPI DirectDrawCreateEx(GUID* g, LPVOID* d, REFIID i, IUnknown* o) {
        LOG_BASIC("DirectDrawCreateEx called");

        if (i != IID_IDirectDraw7) {
            LOG_BASIC("  -> Unsupported interface requested");
            return DDERR_GENERIC;
        }

        m_IDirectDraw7* wrapper = new m_IDirectDraw7();
        *d = wrapper;

        LOG_BASIC("  -> Created IDirectDraw7 wrapper");
        return DD_OK;
    }

    HRESULT WINAPI DirectDrawCreate(GUID* g, LPDIRECTDRAW* d, IUnknown* o) {
        LOG_BASIC("DirectDrawCreate called (legacy, not supported)");
        return DDERR_GENERIC;
    }

}