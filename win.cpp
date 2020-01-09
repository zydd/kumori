#include "win.h"

#include <qmap.h>
#include <qdebug.h>

void play_pause() {
    KEYBDINPUT kbi;
    kbi.wVk     = VK_MEDIA_PLAY_PAUSE;
    kbi.wScan   = 0;
    kbi.dwFlags = 0;
    kbi.time    = 0;
    kbi.dwExtraInfo = ULONG_PTR(GetMessageExtraInfo());

    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki = kbi;

    SendInput(1, &input, sizeof(INPUT));
}

void exclude_from_peek(HWND handle) {
    QMap<HRESULT, QString> hresult_err;
    hresult_err[E_HANDLE] = "Invalid handle";
    hresult_err[S_OK] = "Success";

    BOOL bValue = TRUE;
    auto res = DwmSetWindowAttribute(handle, DWMWA_EXCLUDED_FROM_PEEK, &bValue, sizeof(bValue));
    qDebug() << "exclude_from_peek"
             << hex << uint32_t(res)
             << (hresult_err.contains(res) ? hresult_err[res] : "");
    /* when this flag is set, the window disappears during the
     * animation when changing workspaces
     */
}

void ignore_show_desktop(HWND handle) {
    HWND hwnd = nullptr;

    auto callback = [](HWND hwnd, LPARAM lParam) -> BOOL {
        HWND *ret = reinterpret_cast<HWND*>(lParam);
        *ret = FindWindowEx(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
        return *ret == nullptr;
    };

    EnumWindows(callback, LPARAM(&hwnd));

    SetParent(handle, hwnd);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    HWND p = FindWindowEx(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    HWND *ret = reinterpret_cast<HWND*>(lParam);

    if (p) {
        // Gets the WorkerW Window after the current one.
        *ret = FindWindowEx(nullptr, hwnd, L"WorkerW", nullptr);
        return false;
    }
    return true;
}

HWND get_wallpaper_window() {
    // Fetch the Progman window
    HWND progman = FindWindow(L"ProgMan", nullptr);
    // Send 0x052C to Progman. This message directs Progman to spawn a
    // WorkerW behind the desktop icons. If it is already there, nothing
    // happens.
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    // We enumerate all Windows, until we find one, that has the SHELLDLL_DefView
    // as a child.
    // If we found that window, we take its next sibling and assign it to workerw.
    HWND wallpaper_hwnd = nullptr;
    EnumWindows(EnumWindowsProc, LPARAM(&wallpaper_hwnd));

    return wallpaper_hwnd;
}
