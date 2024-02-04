/**************************************************************************
 *  dirwatcher.h
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

#ifndef DIRWATCHER_H
#define DIRWATCHER_H

#include <qfilesystemwatcher.h>
#include <qurl.h>

class DirWatcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList entries READ entries NOTIFY entriesChanged)
    Q_PROPERTY(QList<QUrl> dirs WRITE setDirs)

public:
    explicit DirWatcher(QObject *parent = nullptr);

    inline QStringList entries() const {
        return m_entries; }

public slots:
    void updateEntries();
    void setDirs(QList<QUrl> dirs);

signals:
    void entriesChanged();

private:
    QStringList m_entries;
    QStringList m_dirs;
    QFileSystemWatcher m_watcher;
};

#endif // DIRWATCHER_H
