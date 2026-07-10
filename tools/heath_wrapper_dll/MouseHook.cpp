#include "MouseHook.h"
#include "KeyboardHook.h" // Подключаем KeyboardHook для проверки состояния
#include "Logger.h"

HHOOK MouseHook::s_hMouseHook = nullptr;
HWND MouseHook::s_hGameWnd = nullptr;
bool MouseHook::s_bIsCaptured = false;

void MouseHook::Install(HWND hWnd) {
    if (s_hMouseHook) return;
    s_hGameWnd = hWnd;

    HMODULE hDll = GetModuleHandle(NULL);
    // Используем WH_MOUSE_LL для глобального низкоуровневого перехвата
    s_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hDll, 0);

    if (s_hMouseHook) {
        LOG_BASIC("MouseHook installed successfully.");
    }
    else {
        LOG_BASIC("Failed to install MouseHook. Error: %d", GetLastError());
    }
}

void MouseHook::Uninstall() {
    if (s_hMouseHook) {
        UnhookWindowsHookEx(s_hMouseHook);
        s_hMouseHook = nullptr;
        ClipCursor(NULL); // Гарантированно отпускаем мышь при выгрузке DLL
        LOG_BASIC("MouseHook uninstalled.");
    }
}

LRESULT CALLBACK MouseHook::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT* pMouse = (MSLLHOOKSTRUCT*)lParam;

        // Проверяем, не нажата ли горячая клавиша для принудительного показа курсора
        bool isForcedVisible = KeyboardHook::IsCursorForcedVisible();

        // Если курсор принудительно показан (отпущен), но мышь все еще захвачена -> освобождаем её
        if (isForcedVisible && s_bIsCaptured) {
            ClipCursor(NULL);
            s_bIsCaptured = false;
            LOG_BASIC("[MouseHook] Mouse released via KeyboardHook toggle.");
        }

        // Захватываем мышь при клике, если она внутри окна и НЕ принудительно показана
        if (wParam == WM_LBUTTONDOWN && !isForcedVisible && s_hGameWnd) {
            RECT clientRect;
            GetClientRect(s_hGameWnd, &clientRect);

            POINT pt = pMouse->pt;
            POINT ptClient = pt;
            ScreenToClient(s_hGameWnd, &ptClient);

            if (PtInRect(&clientRect, ptClient)) {
                POINT topLeft = { clientRect.left, clientRect.top };
                POINT bottomRight = { clientRect.right, clientRect.bottom };
                ClientToScreen(s_hGameWnd, &topLeft);
                ClientToScreen(s_hGameWnd, &bottomRight);

                RECT screenClientRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
                ClipCursor(&screenClientRect);
                s_bIsCaptured = true;
                LOG_BASIC("[MouseHook] Mouse captured in client area on LBUTTONDOWN.");
            }
        }
    }

    return CallNextHookEx(s_hMouseHook, nCode, wParam, lParam);
}