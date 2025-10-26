#include <windows.h>
#include <stdio.h>

static FILE *log;

void mouse_click()
{
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;
    inputs[1].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void mouse_press()
{
    INPUT input = { 0 };

    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    UINT uSent = SendInput(1, &input, sizeof(INPUT));
}

void mouse_release()
{
    INPUT input = { 0 };

    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;

    UINT uSent = SendInput(1, &input, sizeof(INPUT));
}

void TimerProc_click(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime
) {
    mouse_click();
    KillTimer(NULL, idEvent);
}

void TimerProc_press(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime
) {
    mouse_press();
    KillTimer(NULL, idEvent);
}

void TimerProc_release(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime
) {
    mouse_release();
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
            if (wParam == WM_KEYDOWN) {
                fprintf(log, "wm");
            }
            else if (wParam == WM_SYSKEYDOWN) {
                fprintf(log, "sys");
            }

            if (vkCode >= 256) {
                fprintf(log, "vkCode[%d] >= 256.. unexpected\n", vkCode);
                break;
            }
            else if (vkCode == VK_ESCAPE) {
                fprintf(log, "keydown[%d].. quitting\n", vkCode);
                PostQuitMessage(0);
                return TRUE;
            }

            int should_click = 0;
            if (vkCode == 192 && !key_down[vkCode]) {       
                should_click = 1;
            }

            key_down[vkCode] = 1;
            fprintf(log, "keydown[%d] %d, flags[%d]\n", vkCode, i++, pKeyBoard->flags);

            if (vkCode == 192) {
                if (should_click)
                    SetTimer(NULL, 0, 50, TimerProc_click);
                return TRUE;
            }

            break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            if (wParam == WM_KEYUP)
                fprintf(log, "wm");
            else if (wParam == WM_SYSKEYUP)
                fprintf(log, "sys");


            key_down[vkCode] = 0;
            fprintf(log, "keyup[%d] %d, flags[%d]\n", vkCode, i++, pKeyBoard->flags);

            if (vkCode == 192)
                return TRUE;

            break;
        }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() 
{
    fopen_s(&log, "log.txt", "w");
    if (!log) {
        printf("cannot open log file.. exit\n");
        return 0;
    }

    HWND hwnd_console = GetConsoleWindow();
    ShowWindow(hwnd_console, SW_HIDE);
    
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hHook);

    if (log) 
        fclose(log);

    return 0;
}
