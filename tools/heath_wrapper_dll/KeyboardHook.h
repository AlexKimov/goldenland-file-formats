#pragma once
#include <windows.h>

class KeyboardHook {
public:
    static void Install();
    static void Uninstall();

    static bool IsCursorForcedVisible();

private:
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    static HHOOK s_hKeyboardHook;
    static bool s_bForceShowCursor;
    static int s_iToggleKey;
};