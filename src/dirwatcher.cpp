#include "dirwatcher.h"

#include <qdir.h>
#include <qdebug.h>

DirWatcher::DirWatcher(QObject *parent):
    QObject(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &DirWatcher::updateEntries);
}

void DirWatcher::updateEntries() {
    m_entries.clear();

    for (auto const& path : m_dirs) {
        QDir dir(path);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

        for (auto e : dir.entryInfoList())
            m_entries.append(e.filePath());
    }

    emit entriesChanged();
}

void DirWatcher::setDirs(QList<QUrl> dirs) {
    m_dirs.clear();
    auto oldDirs = m_watcher.directories();
    if (! oldDirs.empty())
        m_watcher.removePaths(oldDirs);

    for (auto const& d : dirs) {
        auto path = d.toLocalFile();
        if (QFileInfo(path).isDir())
            m_dirs.push_back(path);
    }

    m_watcher.addPaths(m_dirs);

    updateEntries();
}
