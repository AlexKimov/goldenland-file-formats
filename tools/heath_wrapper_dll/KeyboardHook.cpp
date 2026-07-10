#include "KeyboardHook.h"
#include "Logger.h" // Òגמי כמדדונ
#include "Settings.h"

HHOOK KeyboardHook::s_hKeyboardHook = nullptr;
bool KeyboardHook::s_bForceShowCursor = false;
int KeyboardHook::s_iToggleKey = VK_F12; // 123 = F12

void KeyboardHook::Install() {
    if (s_hKeyboardHook) return;

    s_iToggleKey = Settings::GetInstance().ReleaseMouseKey;

    HMODULE hDll = GetModuleHandle(NULL);
    s_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hDll, 0);

    if (s_hKeyboardHook) {
        LOG_BASIC("KeyboardHook installed successfully. Toggle key: %d", s_iToggleKey);
    }
    else {
        LOG_BASIC("Failed to install KeyboardHook. Error: %d", GetLastError());
    }
}

void KeyboardHook::Uninstall() {
    if (s_hKeyboardHook) {
        UnhookWindowsHookEx(s_hKeyboardHook);
        s_hKeyboardHook = nullptr;
        LOG_BASIC("KeyboardHook uninstalled.");
    }
}

bool KeyboardHook::IsCursorForcedVisible() {
    return s_bForceShowCursor;
}

LRESULT CALLBACK KeyboardHook::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKBStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (pKBStruct->vkCode == (DWORD)s_iToggleKey) {
            s_bForceShowCursor = !s_bForceShowCursor; 

            if (s_bForceShowCursor) {
                LOG_BASIC("[KeyboardHook] Hotkey pressed: Forcing cursor VISIBLE.");
 
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                while (ShowCursor(TRUE) < 0);

                ClipCursor(NULL); 
            }
            else {
                LOG_BASIC("[KeyboardHook] Hotkey pressed: Allowing game to HIDE cursor.");
 
                SetCursor(NULL);
                while (ShowCursor(FALSE) >= 0);
            }
        }
    }

    return CallNextHookEx(s_hKeyboardHook, nCode, wParam, lParam);
}