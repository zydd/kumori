#include "launcher.h"

#include <qdebug.h>
#include <qdesktopservices.h>
#include <qdir.h>
#include <qfilesystemwatcher.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qprocess.h>
#include <qstandardpaths.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>

#include "iconextractor.h"
#endif

Launcher::Launcher(QObject *parent):
    QObject(parent),
    m_iconProvider(new IconProvider(this)),
    m_desktop(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation))
{
    auto watcher = new QFileSystemWatcher(this);
    watcher->addPaths(m_desktop);
    connect(watcher, &QFileSystemWatcher::directoryChanged,
            this, &Launcher::reloadConfig);

    qDebug() << watcher->directories();

    reloadConfig();
}

void Launcher::launch(QString program) {
    auto path = m_shortcuts[program]["path"];
    qDebug() << "launch" << path;

    if (path.isValid()) {
#ifdef Q_OS_WIN
        ShellExecute(nullptr, nullptr, reinterpret_cast<const WCHAR*>(path.toString().utf16()),
                     nullptr, nullptr, SW_NORMAL);
#else
        QStringList args;
        if (m_shortcuts[program].contains("param"))
            args << m_shortcuts[program]["param"].toString();

        qDebug() << path.toString() << args;
        QProcess proc;
        proc.setProgram(path.toString());
        proc.setArguments(args);
        proc.setWorkingDirectory(QFileInfo(proc.program()).dir().path());
        proc.startDetached();
#endif
    }
}

void Launcher::reloadConfig() {
    qDebug() << "Launcher::reloadConfig";

    m_shortcuts.clear();
    m_programs.clear();

    for (auto folder : m_desktop) {
        QDir dir(folder);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::System);

        for (auto e : dir.entryInfoList()) {
            m_programs.append(e.baseName());
            m_shortcuts[e.baseName()]["path"] = e.filePath();
        }
    }

    emit programsChanged();
}

Launcher::IconProvider::IconProvider(Launcher *launcher):
    QQuickImageProvider(QQuickImageProvider::Pixmap),
    launcher(launcher)
{ }

QPixmap Launcher::IconProvider::requestPixmap(const QString &id, QSize *size,
                                              const QSize &/*requestedSize*/) {
    auto icon = launcher->m_shortcuts[id]["icon"];
    if (icon.isValid()) {
        QImage ret(icon.toString());
        *size = ret.size();

        return QPixmap::fromImage(ret);

    } else {
#ifdef Q_OS_WIN
        auto path = launcher->m_shortcuts[id]["path"];
        if (! path.isValid())
            return {};

        auto icons = extractShellIcons(path.toString(), false);

        auto ret = icons[3].pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        *size = ret.size();

        return ret;
#endif
    }
}
