#include "WindowManager.h"
#include "InlineHook.h"
#include "IATHook.h"
#include "Logger.h"
#include "MouseHook.h"
#include "Settings.h"

// Константы
const char* WindowManager::GAME_WINDOW_CLASS = "GX-Class";
const int WindowManager::GAME_WIDTH = 800;
const int WindowManager::GAME_HEIGHT = 600;
const DWORD WindowManager::SUB_4B44E0_RVA = 0x000B44E0;

WindowManager& WindowManager::GetInstance() {
    static WindowManager instance;
    return instance;
}

WindowManager::WindowManager()
    : m_hWnd(nullptr)
    , m_OrigWndProc(nullptr)
    , m_MouseLocked(true)
    , m_TargetWindowWidth(GAME_WIDTH)
    , m_TargetWindowHeight(GAME_HEIGHT)
    , m_OrigCreateWindowExA(nullptr)
    , m_OrigSetCursor(nullptr)
    , m_OrigSub4B44E0(nullptr)
    , m_OrigShowCursor(nullptr)
    , m_Sub4B44E0HookInfo(nullptr)
    , m_OrigDefWindowProcA(nullptr)
{
}

WindowManager::~WindowManager() {
    Shutdown();
}

bool WindowManager::Initialize() {
    if (!Settings::GetInstance().bEnabled) {
        LOG_BASIC("Wrapper is DISABLED in wrapper.ini. Skipping WindowManager hooks.");
        return true; 
    }

    LOG_BASIC("WindowManager initializing...");

    if (!InstallIATHooks()) {
        LOG_BASIC("Failed to install IAT hooks");
        return false;
    }

    if (!InstallInlineHooks()) {
        LOG_BASIC("Failed to install inline hooks");
        RemoveIATHooks();
        return false;
    }

    ApplyMemoryPatches();

    LOG_BASIC("WindowManager initialized successfully");
    return true;
}

void WindowManager::Shutdown() {
    RemoveInlineHooks();
    RemoveIATHooks();
    MouseHook::Uninstall();
    LOG_BASIC("WindowManager shutdown complete");
}

bool WindowManager::InstallIATHooks() {
    LOG_EXT("Installing IAT hooks...");

    static WindowManager* s_Instance = this;

    if (!IATHook::Patch("USER32.dll", "CreateWindowExA",
        (void*)Hooked_CreateWindowExA, (void**)&m_OrigCreateWindowExA)) {
        LOG_BASIC("Failed to hook CreateWindowExA");
    }

    if (!IATHook::Patch("USER32.dll", "SetCursor",
        (void*)Hooked_SetCursor, (void**)&m_OrigSetCursor)) {
        LOG_BASIC("Failed to hook SetCursor");
    }

    if (!IATHook::Patch("USER32.dll", "DefWindowProcA", (void*)Hooked_DefWindowProcA, (void**)&m_OrigDefWindowProcA)) {
        LOG_BASIC("Failed to hook DefWindowProcA");
    }

    if (!IATHook::Patch("USER32.dll", "ShowCursor", (void*)Hooked_ShowCursor, (void**)&m_OrigShowCursor)) {
        LOG_BASIC("Failed to hook ShowCursor");
    }

    LOG_EXT("IAT hooks installed successfully");
    return true;
}


bool WindowManager::InstallInlineHooks() {
    if (!Settings::GetInstance().bEnabled) {
        LOG_BASIC("Skipping InlineHooks (Wrapper Disabled).");
        return true;
    }

    HMODULE hGame = GetModuleHandleA(NULL);
    if (!hGame) return false;

    void* targetAddr = (void*)((DWORD)hGame + SUB_4B44E0_RVA);

    // InlineHook возвращает HookInfo*, сохраняем его
    m_Sub4B44E0HookInfo = InlineHook::Install(targetAddr, (void*)Hooked_Sub4B44E0, 5);

    if (m_Sub4B44E0HookInfo) {
        InlineHook::HookInfo* info = (InlineHook::HookInfo*)m_Sub4B44E0HookInfo;
        m_OrigSub4B44E0 = (tSub4B44E0)info->trampoline;
        LOG_BASIC("Successfully hooked sub_4B44E0 at %p", targetAddr);
        return true;
    }

    LOG_BASIC("Failed to hook sub_4B44E0");
    return false;
}

void WindowManager::RemoveInlineHooks() {
    if (m_Sub4B44E0HookInfo) {
        InlineHook::Remove((InlineHook::HookInfo*)m_Sub4B44E0HookInfo);
        m_Sub4B44E0HookInfo = nullptr;
        m_OrigSub4B44E0 = nullptr;
    }
}

void WindowManager::RemoveIATHooks() {
    m_OrigCreateWindowExA = nullptr;
    m_OrigSetCursor = nullptr;

    LOG_EXT("IAT hooks cleanup complete");
}


HWND WindowManager::Hooked_CreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X, int Y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    WindowManager& mgr = GetInstance();

    if (lpClassName && strcmp(lpClassName, GAME_WINDOW_CLASS) == 0) {
        LOG_BASIC("CreateWindowExA intercepted for %s", GAME_WINDOW_CLASS);

        dwStyle &= ~WS_POPUP;
        dwStyle &= ~WS_POPUPWINDOW;
        dwStyle |= WS_OVERLAPPEDWINDOW;
        dwExStyle &= ~WS_EX_TOPMOST;
        dwStyle &= ~WS_THICKFRAME;    
        dwStyle &= ~WS_MAXIMIZEBOX;  

        nWidth = GAME_WIDTH;
        nHeight = GAME_HEIGHT;

        RECT rc = { 0, 0, nWidth, nHeight };
        AdjustWindowRect(&rc, dwStyle, FALSE);
        nWidth = rc.right - rc.left;
        nHeight = rc.bottom - rc.top;

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        X = (screenW - nWidth) / 2;
        Y = (screenH - nHeight) / 2;

        mgr.m_TargetWindowWidth = nWidth;
        mgr.m_TargetWindowHeight = nHeight;

        LOG_BASIC("Forced client area to %dx%d, outer size: %dx%d at (%d,%d)",
            GAME_WIDTH, GAME_HEIGHT, nWidth, nHeight, X, Y);
    }

    HWND hWnd = mgr.m_OrigCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    if (hWnd) {
        LOG_BASIC("Window created: %p", hWnd);
        LOG_BASIC("SetCursor hook installed: %p", mgr.m_OrigSetCursor);
    }

    if (lpClassName && strcmp(lpClassName, GAME_WINDOW_CLASS) == 0 && hWnd) {
        mgr.m_hWnd = hWnd;

        if (!mgr.m_OrigWndProc) {
            mgr.m_OrigWndProc = (WNDPROC)SetWindowLongPtrA(hWnd, GWLP_WNDPROC,
                (LONG_PTR)Hooked_WndProc);
            LOG_BASIC("WndProc hooked successfully");
        }

        if (Settings::GetInstance().bEnabled && Settings::GetInstance().LockMouse) {
            MouseHook::Install(hWnd);
        }

        LOG_BASIC("Main window created: %p", hWnd);
    }

    return hWnd;
}

LRESULT CALLBACK WindowManager::Hooked_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WindowManager& mgr = GetInstance();

    switch (msg) {
        case WM_WINDOWPOSCHANGING: {
            WINDOWPOS* pos = (WINDOWPOS*)lParam;
            if (!(pos->flags & SWP_NOSIZE)) {
                pos->cx = mgr.m_TargetWindowWidth;
                pos->cy = mgr.m_TargetWindowHeight;
            }
            LOG_BASIC("WM_WINDOWPOSCHANGING");
            break;
        }

        case WM_ERASEBKGND: {
            LOG_BASIC("WM_ERASEBKGND");
            return 1;
        }
    }

    if (mgr.m_OrigWndProc) {
        return CallWindowProcA(mgr.m_OrigWndProc, hWnd, msg, wParam, lParam);
    }
    
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// fix double WM_NCCALCSIZE call that makes client area smaller
int __stdcall WindowManager::Hooked_Sub4B44E0(int arg0, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    WindowManager& mgr = GetInstance();

    if (Msg == WM_NCCALCSIZE || Msg == WM_SIZE || Msg == WM_GETMINMAXINFO) {
        return 0;
    }

    return mgr.m_OrigSub4B44E0(arg0, hWnd, Msg, wParam, lParam);
}

static bool s_IsMouseInClient = true;


HCURSOR WINAPI WindowManager::Hooked_SetCursor(HCURSOR hCursor) {
    WindowManager& mgr = GetInstance();
    if (!mgr.m_hWnd) return mgr.m_OrigSetCursor(hCursor);

    POINT pt;
    GetCursorPos(&pt);
    RECT clientRect;
    GetClientRect(mgr.m_hWnd, &clientRect);
    POINT ptClient = pt;
    ScreenToClient(mgr.m_hWnd, &ptClient);
    bool inClient = PtInRect(&clientRect, ptClient);

    
    if (inClient != s_IsMouseInClient) {
        s_IsMouseInClient = inClient;
        if (!inClient) {
            while (mgr.m_OrigShowCursor(TRUE) < 0);
            LOG_BASIC("-> Mouse left client area. Forced Windows cursor visible.");
        }
        else {
            while (mgr.m_OrigShowCursor(FALSE) >= 0);
            LOG_BASIC("-> Mouse entered client area. Hiding Windows cursor for game.");
        }
    }

    if (inClient) {
        return mgr.m_OrigSetCursor(NULL);
    }

    return mgr.m_OrigSetCursor(LoadCursor(NULL, IDC_ARROW));
}

void WindowManager::ApplyMemoryPatches()
{
    HMODULE hGame = GetModuleHandleA(NULL);
    if (!hGame) return;

    int scrollDelay = Settings::GetInstance().iMapScrollDelay;

    // make game scroll slower
    if (scrollDelay >= 0) {
        // "cmp eax, 37h" по адресу 0x4DB1E5
        // RVA = 0xDB1E5
        DWORD cmpInstructionAddr = (DWORD)hGame + 0xDB1E5;
        DWORD oldProtect;

        if (VirtualProtect((void*)cmpInstructionAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            DWORD originalValue = *(DWORD*)(cmpInstructionAddr + 1);

            *(DWORD*)(cmpInstructionAddr + 1) = (DWORD)scrollDelay;

            VirtualProtect((void*)cmpInstructionAddr, 5, oldProtect, &oldProtect);

            LOG_BASIC("Patched Map Scroll Delay at 0x%08X: 0x%X -> 0x%X",
                cmpInstructionAddr, originalValue, scrollDelay);
        }
    }
    else {
        LOG_BASIC("MapScrollDelay is set to -1, skipping memory patch.");
    }
}

int WINAPI WindowManager::Hooked_ShowCursor(BOOL bShow) {
    WindowManager& mgr = GetInstance();

    if (!s_IsMouseInClient) {
        if (!bShow) {
            int c = mgr.m_OrigShowCursor(TRUE);
            mgr.m_OrigShowCursor(FALSE);
            return c;
        }
    }

    return mgr.m_OrigShowCursor(bShow);
}


LRESULT WINAPI WindowManager::Hooked_DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    WindowManager& mgr = GetInstance();

    if (Msg == WM_CLOSE) { // game window close button clicked 
        LOG_BASIC(">>> WM_CLOSE caught! Triggering graceful exit.");

        HMODULE hGame = GetModuleHandle(NULL);
        if (hGame) {
            DWORD** ppMenuObj = (DWORD**)((BYTE*)hGame + 0x001016F0);

            if (ppMenuObj && *ppMenuObj) {
                DWORD* pMenuObj = *ppMenuObj;

                LOG_BASIC("Menu object found at %p. Setting menu loop break flags.", pMenuObj);

                pMenuObj[1] = 5; //  flag that start menu exit button clicked 
                pMenuObj[16] = 1; // flag to destroy menu
            }
            else {
                LOG_BASIC("Menu object pointer is NULL (probably in gameplay or not initialized).");
            }
        }

        return 0;
    }

    return mgr.m_OrigDefWindowProcA(hWnd, Msg, wParam, lParam);
}

