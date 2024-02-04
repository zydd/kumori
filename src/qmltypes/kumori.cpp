/**************************************************************************
 *  kumori.cpp
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

#include "kumori.h"

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdesktopservices.h>
#include <qdir.h>
#include <qfilesystemwatcher.h>
#include <qqmlengine.h>
#include <qquickwindow.h>
#include <qscreen.h>
#include <qstandardpaths.h>
#include <qwinfunctions.h>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>
#endif

Kumori *Kumori::m_instance = nullptr;

Kumori::Kumori(QStringList args):
    QQmlPropertyMap(this, nullptr),
    m_args(args)
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    auto appDir = QDir::fromNativeSeparators(qApp->applicationDirPath());
    addConfig("currentDir", QDir::currentPath());
    addConfig("appDir", appDir);
    addConfig("appImportDir", appDir + "/qml");
    addConfig("userImportDir",
              QStringLiteral("%1/%2/desktop")
              .arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
              .arg(qApp->applicationName()));
    addConfig("monitoredFileTypes", QStringList{"*.qml", "*.vert", "*.frag"});
}

QVariant Kumori::config(const QString &key) {
    auto keys = key.split('.');

    QQmlPropertyMap *obj = instance();

    for (int i = 0; i < keys.size() - 1 && obj; ++i)
        obj = obj->value(keys[i]).value<QQmlPropertyMap *>();

    return obj ? obj->value(keys.last()) : QVariant();
}

void Kumori::ignoreAeroPeek(QQuickWindow *window) {
#ifdef Q_OS_WIN
    /* when this flag is set, the window disappears during the
     * animation when changing workspaces
     */
    BOOL bValue = TRUE;
    auto hr = DwmSetWindowAttribute(HWND(window->winId()), DWMWA_EXCLUDED_FROM_PEEK, &bValue, sizeof(bValue));
    if (FAILED(hr)) {
        qDebug() << "Kumori::ignoreAeroPeek()"
                 << Qt::hex << unsigned(hr)
                 << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
    }
#endif
}

void Kumori::drawOverDesktop(QQuickWindow *window) {
#ifdef Q_OS_WIN
    HWND desktop = nullptr;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        HWND *ret = reinterpret_cast<HWND *>(lParam);
        *ret = FindWindowEx(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
        return *ret == nullptr;
    }, LPARAM(&desktop));

    if (!desktop) {
        qDebug() << "Kumori::drawOverDesktop(): unable to locate desktop window";
        return;
    }

    auto desktopWindow = QWindow::fromWinId(reinterpret_cast<WId>(desktop));
    window->setParent(desktopWindow);
#endif
}

void Kumori::drawUnderDesktop(QQuickWindow *window) {
#ifdef Q_OS_WIN
    // Spawn a WorkerW behind the desktop icons.
    SendMessageTimeout(FindWindow(L"Progman", nullptr), 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    HWND workerw = nullptr;

    // Find window that has the SHELLDLL_DefView as a child and take its next sibling.
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (FindWindowEx(hwnd, nullptr, L"SHELLDLL_DefView", nullptr)) {
            HWND *ret = reinterpret_cast<HWND *>(lParam);
            // return the WorkerW window after SHELLDLL_DefView.
            *ret = FindWindowEx(nullptr, hwnd, L"WorkerW", nullptr);
            return false;
        }
        return true;
    }, LPARAM(&workerw));

    if (!workerw) {
        qDebug() << "Kumori::drawOverDesktop(): unable to locate wallpaper window";
        return;
    }

    auto desktopWindow = QWindow::fromWinId(reinterpret_cast<WId>(workerw));
    window->setParent(desktopWindow);
#endif
}

QByteArray Kumori::readFile(QUrl url) {
    if (url.isLocalFile()) {
        QFile file(url.toLocalFile());
        if (file.open(QFile::ReadOnly))
            return file.readAll();
    }

    qWarning() << "Kumori::readFile(): failed to open file" << url;
    return {};
}

bool Kumori::appendLog(QUrl url, QString text) {
    qDebug() << url;
    auto ext = url.toLocalFile().split('.').last();

    if (ext != "txt" && ext != "log") {
        qWarning() << "this method can only be used for txt/log files:" << ext;
        return false;
    }

    QFile file(url.toLocalFile());
    if (!file.open(QFile::WriteOnly | QFile::Append)) {
        qDebug() << "failed to open file" << url;
        return false;
    }

    return file.write(text.toUtf8()) >= 0;
}


void Kumori::clearComponentCache() {
    if (m_engine)
        m_engine->clearComponentCache();
    else
        qDebug() << "Kumori::clearComponentCache(): engine unavailable";
}

void Kumori::playPause() {
#ifdef Q_OS_WIN
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
#endif
}

void Kumori::addConfig(QString const& key, QVariant const& defaultValue, bool setDefault, bool ignoreArgs) {
    if (key.isEmpty())
        return;

    auto config = m_config.value("config/" + key);

    if (config.isNull() && !defaultValue.isNull()) {
        config = defaultValue;

        if (setDefault)
            m_config.setValue("config/" + key, config);
    }

    if (!ignoreArgs) {
        auto argName = QStringLiteral("--%1=").arg(key);

        for (auto const& arg : m_args)
            if (arg.startsWith(argName))
                config = arg.mid(argName.size());
    }

    auto keys = key.split('.');

    QQmlPropertyMap *obj = this;
    for (int i = 0; i < keys.size() - 1; ++i) {
        if (obj->contains(keys[i]) && obj->value(keys[i]).type() == qMetaTypeId<QObject *>()) {
            obj = obj->value(keys[i]).value<QQmlPropertyMap *>();
        } else {
            auto sub = new Group(keys.mid(0, i+1).join('.'));
            obj->insert(keys[i], QVariant::fromValue<QObject *>(sub));
            obj = sub;
        }
    }

    obj->insert(keys.last(), config);
}

QVariant Kumori::updateValue(const QString &key, const QVariant &input) {
    m_config.setValue("config/" + key, input);
    return input;
}

QVariant Kumori::Group::updateValue(const QString &key, const QVariant &input) {
    m_config.setValue(QStringLiteral("config/%1.%2").arg(m_prefix).arg(key), input);
    return input;
}

void Kumori::hideTaskbar() {
    auto taskbar = FindWindow(L"Shell_TrayWnd", NULL);
    ShowWindow(taskbar, SW_HIDE);

//    APPBARDATA abd = {};
//    abd.cbSize = sizeof(APPBARDATA);
//    abd.hWnd = taskbar;
//    abd.lParam = ABS_AUTOHIDE;

//    auto res = SHAppBarMessage(ABM_SETSTATE, &abd);
//    if (!res) {
//        qWarning() << "could not hide taskbar";
//    }

    SetWindowPos(taskbar, HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Kumori::showTaskbar() {
    auto taskbar = FindWindow(L"Shell_TrayWnd", NULL);
    ShowWindow(taskbar, SW_SHOW);
    GetParent(taskbar);
}

void ShellKeyCombo(std::initializer_list<WORD> keys) {
    QVector<INPUT> inputs;
    auto info = GetMessageExtraInfo();

    for (WORD key : keys) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.dwExtraInfo = info;
        input.ki.wVk = key;

        inputs.push_back(input);
    }

    for (auto itr = std::reverse_iterator(keys.end()); itr != std::reverse_iterator(keys.begin()); ++itr) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.dwExtraInfo = info;
        input.ki.wVk = *itr;
        input.ki.dwFlags = KEYEVENTF_KEYUP;

        inputs.push_back(input);
    }

    SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}

void Kumori::actionCenter() {
    ShellKeyCombo({VK_LWIN, 'A'});
}

void Kumori::notificationArea() {
    ShellKeyCombo({VK_LWIN, 'N'});
}

void Kumori::startMenu() {
    ShellKeyCombo({VK_LWIN});
}


// Blur window

enum WINDOWCOMPOSITIONATTRIB {
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_LAST = 26
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4, // RS4 1803
    ACCENT_ENABLE_HOSTBACKDROP = 5, // RS5 1809
    ACCENT_INVALID_STATE = 6
};

struct ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

typedef BOOL (WINAPI *pfnGetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
typedef BOOL (WINAPI *pfnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);


void Kumori::blurWindowBackground(QQuickWindow *window) {
    auto hwnd = reinterpret_cast<HWND>(window->winId());

    static HMODULE hUser = GetModuleHandle(L"user32.dll");
    static auto setWindowCompositionAttribute = pfnSetWindowCompositionAttribute(
                GetProcAddress(hUser, "SetWindowCompositionAttribute"));

    ACCENT_POLICY accent = { ACCENT_ENABLE_BLURBEHIND, 0, 0, 0 };
    WINDOWCOMPOSITIONATTRIBDATA data;
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &accent;
    data.cbData = sizeof(accent);
    setWindowCompositionAttribute(hwnd, &data);
}
