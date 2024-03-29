/**************************************************************************
 *  nativewindow.cpp
 *
 *  Copyright 2024 Gabriel Machado
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 **************************************************************************/

#include "nativewindow.h"

#include <qcoreevent.h>
#include <qdebug.h>
#include <qoperatingsystemversion.h>
#include <qwinfunctions.h>

#include <Windows.h>
#include <dwmapi.h>
#include <processthreadsapi.h>
#include <winuser.h>

#include "liveicon.h"


static QString getProcessName(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid) {
        // open process
        // QueryLimitedInformation flag allows us to access elevated applications as well
        auto hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);

        // get path
        WCHAR _processName[1024];
        DWORD processNameLength = sizeof(_processName) / sizeof(*_processName);
        QueryFullProcessImageName(hProc, 0, _processName, &processNameLength);
        return QString::fromWCharArray(_processName, processNameLength);
    } else {
        qWarning() << "could not get process name for window" << hwnd;
    }

    return {};
}


NativeWindow::NativeWindow(HWND hwnd, QObject *parent)
    : QObject{parent}, _hwnd{hwnd}
{
    qDebug() << _hwnd;
    _icon = new LiveIcon();

    WCHAR windowClass[256];
    GetClassName(_hwnd, windowClass, sizeof(windowClass) / sizeof(*windowClass));
    _windowClass = QString::fromWCharArray(windowClass);

    _processName = getProcessName(_hwnd);

    if (_windowClass == "Windows.UI.Core.CoreWindow") {
        if (_processName.endsWith("SearchHost.exe"))
            _role = StartMenuWindow;
        else if (_processName.endsWith("ShellExperienceHost.exe"))
            _role = ShellWindow;
    }
}

NativeWindow::~NativeWindow() {
    qDebug() << _hwnd;
    delete _icon;
}


bool NativeWindow::canAddToTaskbar() {
    int extendedWindowStyles = GetWindowLong(_hwnd, GWL_EXSTYLE);
    bool isWindow = IsWindow(_hwnd);
    bool isVisible = IsWindowVisible(_hwnd);
    bool isToolWindow = (extendedWindowStyles & WS_EX_TOOLWINDOW) != 0;
    bool isAppWindow = (extendedWindowStyles & WS_EX_APPWINDOW) != 0;
    bool isNoActivate = (extendedWindowStyles & WS_EX_NOACTIVATE) != 0;
    auto ownerWin = GetWindow(_hwnd, GW_OWNER);

    bool canShow = isWindow && isVisible && (ownerWin == nullptr || isAppWindow)
            && (!isNoActivate || isAppWindow) && !isToolWindow;

    if (!canShow) {
        qDebug() << "hidden window:" << _hwnd << title();
        return false;
    }

    // UWP shell windows that are not cloaked should be hidden from the taskbar
    if (_windowClass == "ApplicationFrameWindow" || _windowClass == "Windows.UI.Core.CoreWindow") {
        if ((GetWindowLong(_hwnd, GWL_EXSTYLE) & WS_EX_WINDOWEDGE) == 0) {
            qDebug() << "shell window:" << _hwnd << title().toUtf8() << _windowClass << _processName;
            return false;
        }
    }

//    if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10
//               && (windowClass == "ImmersiveBackgroundWindow"
//                   || windowClass == "SearchPane"
//                   || windowClass == "NativeHWNDHost"
//                   || windowClass == "Shell_CharmWindow"
//                   || windowClass == "ImmersiveLauncher")
//               && processName.toLower().endsWith("explorer.exe")
//            ) {
//        qDebug() << "immersive shell window: " << _hwnd << title();
//        return false;
//    }

    qDebug() << "visible window:" << _hwnd << title();
    return true;
}

bool NativeWindow::cloaked() {
    BOOL cloaked = false;
    auto succeeded = SUCCEEDED(DwmGetWindowAttribute(_hwnd, DWMWA_CLOAKED, &cloaked, sizeof(BOOL)));

    qDebug() << succeeded << cloaked << _hwnd << title();

    return succeeded && cloaked;
}


QString NativeWindow::title() {
    wchar_t title[1024];
    GetWindowTextW(_hwnd, title, sizeof(title) / sizeof(*title));
    return QString::fromWCharArray(title);
}


void NativeWindow::toFront() {
    if (minimized()) {
        restore();
    } else {
        ShowWindow(_hwnd, SW_SHOW);
        makeForeground();

//        // some stubborn windows (Outlook) start flashing while already active, this lets us stop
//        if (State == WS_FLASHING)
//            State = WS_ACTIVE;
    }
}


void NativeWindow::minimize() {
    if (GetWindowLong(_hwnd, GWL_STYLE) & WS_MINIMIZEBOX) {
        DWORD retval = NULL;
        SendMessageTimeout(_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0, 2, 200, &retval);
        setActive(false);
    }
}


void NativeWindow::restore() {
    DWORD retval = NULL;
    SendMessageTimeout(_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0, 2, 200, &retval);

    makeForeground();
}


void NativeWindow::maximize() {
    bool maximizeResult = ShowWindow(_hwnd, WS_MAXIMIZE);
    if (!maximizeResult) {
        // we don't have a fallback for elevated windows here since our only hope, SC_MAXIMIZE, doesn't seem to work for them. fall back to restore.
        DWORD retval = NULL;
        SendMessageTimeout(_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0, 2, 200, &retval);
    }
    makeForeground();
}


void NativeWindow::makeForeground() {
    qDebug() << _hwnd;
    SetForegroundWindow(GetLastActivePopup(_hwnd));
}


void NativeWindow::loadIcon() {
    qDebug() << _hwnd;

    HICON hIcon = (HICON)SendMessage(_hwnd, WM_GETICON, ICON_SMALL2, 0);
    if (hIcon == NULL) // If the window does not have an icon, try to get the icon from its class.
        hIcon = (HICON)GetClassLongPtr(_hwnd, GCLP_HICONSM);
    if (hIcon == NULL) // If the class does not have an icon, get the default icon.
        hIcon = LoadIcon(NULL, IDI_APPLICATION);

    // FIXME: WinRT app icons

    auto pixmap = QtWin::fromHICON(hIcon);
    _icon->setIcon(pixmap);
}


bool NativeWindow::minimized() {
    return IsIconic(_hwnd);
}


int NativeWindow::showStyle() {
    WINDOWPLACEMENT placement;
    GetWindowPlacement(_hwnd, &placement);
    return placement.showCmd;
}


void NativeWindow::setActive(bool newActive) {
    if (_active == newActive)
        return;

    _active = newActive;
    qDebug() << "activeChanged";
    emit activeChanged();
}
