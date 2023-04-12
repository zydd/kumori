#include "nativewindow.h"

#include <qcoreevent.h>
#include <qdebug.h>
#include <qoperatingsystemversion.h>

#include <Windows.h>
#include <dwmapi.h>
#include <processthreadsapi.h>
#include <winuser.h>

NativeWindow::NativeWindow(HWND hwnd, QObject *parent)
    : QObject{parent}, hwnd{hwnd}
{
    startTimer(30000);
}


bool NativeWindow::canAddToTaskbar() {
    int extendedWindowStyles = GetWindowLong(hwnd, GWL_EXSTYLE);
    bool isWindow = IsWindow(hwnd);
    bool isVisible = IsWindowVisible(hwnd);
    bool isToolWindow = (extendedWindowStyles & WS_EX_TOOLWINDOW) != 0;
    bool isAppWindow = (extendedWindowStyles & WS_EX_APPWINDOW) != 0;
    bool isNoActivate = (extendedWindowStyles & WS_EX_NOACTIVATE) != 0;
    auto ownerWin = GetWindow(hwnd, GW_OWNER);

    bool canShow = isWindow && isVisible && (ownerWin == nullptr || isAppWindow)
            && (!isNoActivate || isAppWindow) && !isToolWindow;

    if (!canShow)
        return false;

    // >=Win8
    if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows8)
        return false;

    BOOL cloaked = false;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(BOOL));

    if (cloaked) {
        qDebug() << "cloaked window:" << hwnd << title();
        return false;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    QString processName;
    if (pid) {
        // open process
        // QueryLimitedInformation flag allows us to access elevated applications as well
        auto hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);

        // get path
        WCHAR _processName[1024];
        DWORD processNameLength = sizeof(_processName) / sizeof(*_processName);
        QueryFullProcessImageName(hProc, 0, _processName, &processNameLength);
        processName = QString::fromWCharArray(_processName, processNameLength);
    }

    // UWP shell windows that are not cloaked should be hidden from the taskbar, too.
    WCHAR _windowClass[256];
    GetClassName(hwnd, _windowClass, sizeof(_windowClass) / sizeof(*_windowClass));
    auto windowClass = QString::fromWCharArray(_windowClass);
    if (windowClass == "ApplicationFrameWindow" || windowClass == "Windows.UI.Core.CoreWindow") {
        if ((GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_WINDOWEDGE) == 0) {
            qDebug() << "UWP non-window:" << hwnd << title();
            return false;
        }
    } else if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10
               && (windowClass == "ImmersiveBackgroundWindow"
                   || windowClass == "SearchPane"
                   || windowClass == "NativeHWNDHost"
                   || windowClass == "Shell_CharmWindow"
                   || windowClass == "ImmersiveLauncher")
               && processName.toLower().contains("explorer.exe")
            ) {
        qDebug() << "immersive shell window: " << title();
        return false;
    }

    return true;
}

QString NativeWindow::title() {
    wchar_t title[1024];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(*title));
    return QString::fromWCharArray(title);
}

void NativeWindow::toFront() {
    if (minimized()) {
        restore();
    } else {
        ShowWindow(hwnd, SW_SHOW);
        makeForeground();

//        // some stubborn windows (Outlook) start flashing while already active, this lets us stop
//        if (State == WS_FLASHING)
//            State = WS_ACTIVE;
    }
}

void NativeWindow::minimize() {
    if (GetWindowLong(hwnd, GWL_STYLE) & WS_MINIMIZEBOX)
    {
        DWORD retval = NULL;
        SendMessageTimeout(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0, 2, 200, &retval);
    }
}

void NativeWindow::restore() {
    DWORD retval = NULL;
    SendMessageTimeout(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0, 2, 200, &retval);

    makeForeground();
}

void NativeWindow::maximize() {
    bool maximizeResult = ShowWindow(hwnd, WS_MAXIMIZE);
    if (!maximizeResult) {
        // we don't have a fallback for elevated windows here since our only hope, SC_MAXIMIZE, doesn't seem to work for them. fall back to restore.
        DWORD retval = NULL;
        SendMessageTimeout(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0, 2, 200, &retval);
    }
    makeForeground();
}

void NativeWindow::makeForeground() {
    SetForegroundWindow(GetLastActivePopup(hwnd));
}

bool NativeWindow::minimized() {
    return IsIconic(hwnd);
}

int NativeWindow::showStyle() {
    WINDOWPLACEMENT placement;
    GetWindowPlacement(hwnd, &placement);
    return placement.showCmd;
}
