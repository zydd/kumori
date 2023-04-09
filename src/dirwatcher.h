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
