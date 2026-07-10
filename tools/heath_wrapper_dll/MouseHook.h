#pragma once
#include <windows.h>

class MouseHook {
public:
    static void Install(HWND hWnd);
    static void Uninstall();

private:
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

    static HHOOK s_hMouseHook;
    static HWND s_hGameWnd;
    static bool s_bIsCaptured;
};