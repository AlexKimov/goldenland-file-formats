#pragma once
#include <windows.h>

class WindowManager {
public:
    static WindowManager& GetInstance();

    bool Initialize();

    void Shutdown();

private:
    WindowManager();
    ~WindowManager();

    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    static HWND WINAPI Hooked_CreateWindowExA(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
        DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
        HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

    static LRESULT CALLBACK Hooked_WndProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static int __stdcall Hooked_Sub4B44E0(
        int arg0, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    bool InstallIATHooks();
    bool InstallInlineHooks();

    void RemoveIATHooks();
    void RemoveInlineHooks();

    HWND m_hWnd;
    WNDPROC m_OrigWndProc;
    bool m_MouseLocked;
    int m_TargetWindowWidth;
    int m_TargetWindowHeight;

    typedef LRESULT(WINAPI* DefWindowProcA_t)(HWND, UINT, WPARAM, LPARAM);
    DefWindowProcA_t m_OrigDefWindowProcA;
    static LRESULT WINAPI Hooked_DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    typedef HCURSOR(WINAPI* SetCursor_t)(HCURSOR);
    SetCursor_t m_OrigSetCursor;

    typedef int (WINAPI* ShowCursor_t)(BOOL bShow);
    ShowCursor_t m_OrigShowCursor;

    static int WINAPI Hooked_ShowCursor(BOOL bShow);
    static HCURSOR WINAPI Hooked_SetCursor(HCURSOR hCursor);

    typedef HWND(WINAPI* CreateWindowExA_t)(DWORD, LPCSTR, LPCSTR, DWORD,
        int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
    CreateWindowExA_t m_OrigCreateWindowExA;

    typedef int(__stdcall* tSub4B44E0)(int, HWND, UINT, WPARAM, LPARAM);
    tSub4B44E0 m_OrigSub4B44E0;

    void* m_Sub4B44E0HookInfo;

    static const char* GAME_WINDOW_CLASS;
    static const int GAME_WIDTH;
    static const int GAME_HEIGHT;
    static const DWORD SUB_4B44E0_RVA;

    void ApplyMemoryPatches();
};