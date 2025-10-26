#include <windows.h>
#include <stdio.h>

void TimerProc_click(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime
) {
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;
    inputs[1].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    KillTimer(NULL, idEvent);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static int i = 0;
    static int key_down[256] = { 0 };

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyBoard->vkCode;

        switch (wParam) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if (wParam == WM_KEYDOWN)
                printf("wm");
            else if (wParam == WM_SYSKEYDOWN)
                printf("sys");

            if (vkCode >= 256) {
                printf("vkCode[%d] >= 256.. unexpected\n", vkCode);
                break;
            }

            int should_click = 0;
            if (vkCode == 192 && !key_down[vkCode])
                should_click = 1;

            key_down[vkCode] = 1;
            printf("keydown[%d] %d\n", vkCode, i++);

            if (should_click) {
                SetTimer(NULL, 0, 50, TimerProc_click);
                return TRUE;
            }

            break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            if (wParam == WM_KEYUP)
                printf("wm");
            else if (wParam == WM_SYSKEYUP)
                printf("sys");


            key_down[vkCode] = 0;
            printf("keyup[%d]\n", vkCode);

            break;
        }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hHook);
    return 0;
}
