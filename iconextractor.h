#ifndef ICONEXTRACTOR_H
#define ICONEXTRACTOR_H

#include <QPixmap>
#include <QStringList>

struct PixmapEntry {
    QString name;
    QPixmap pixmap;
};

typedef QList<PixmapEntry> PixmapEntryList;

QString formatSize(const QSize &size);
PixmapEntryList extractIcons(const QString &sourceFile, bool large);
PixmapEntryList extractShellIcons(const QString &sourceFile, bool addOverlays);

#endif // ICONEXTRACTOR_H
