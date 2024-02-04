/**************************************************************************
 *  dirwatcher.cpp
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
