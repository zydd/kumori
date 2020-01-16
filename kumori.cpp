#include "kumori.h"

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdesktopservices.h>
#include <qfilesystemwatcher.h>
#include <qqmlengine.h>
#include <qquickwindow.h>
#include <qstandardpaths.h>
#include <qwinfunctions.h>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#endif

Kumori *Kumori::m_instance = nullptr;

Kumori::Kumori(QStringList args):
    QQmlPropertyMap(this, nullptr),
    m_args(args)
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    addConfig("appImportDir", qApp->applicationDirPath() + "/qml");
    addConfig("userImportDir",
              QStringLiteral("%1/%2/desktop")
              .arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
              .arg(qApp->applicationName()));
}

QString Kumori::config(QString const& key) {
    auto keys = key.split('.');

    QQmlPropertyMap *obj = instance();

    for (int i = 0; i < keys.size() - 1 && obj; ++i)
        obj = obj->value(keys[i]).value<QQmlPropertyMap *>();

    return obj ? obj->value(keys.last()).toString() : QString();
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
                 << hex << unsigned(hr)
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

    SetParent(HWND(window->winId()), desktop);
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

    SetParent(HWND(window->winId()), workerw);
#endif
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

void Kumori::addConfig(QString key, QVariant const& defaultValue, bool setDefault, bool ignoreArgs) {
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
        if (obj->contains(keys[i])) {
            Q_ASSERT_X(obj->value(keys[i]).type() == qMetaTypeId<QObject *>(),
                       "Kumori::addConfig()", obj->value(keys[i]).typeName());

            obj = obj->value(keys[i]).value<QQmlPropertyMap *>();
        } else {
            auto sub = new QQmlPropertyMap;
            obj->insert(keys[i], QVariant::fromValue<QObject *>(sub));
            obj = sub;
        }
    }

    Q_ASSERT(!obj->contains(keys.last()));

    obj->insert(keys.last(), config);
}
