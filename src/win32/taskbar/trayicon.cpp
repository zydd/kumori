#include "trayicon.h"

#include <qpainter.h>

void TrayIconPainter::setTrayIconObject(QObject *newTrayIcon) {
    _trayIcon = qobject_cast<TrayIcon *>(newTrayIcon);

    connect(_trayIcon, &TrayIcon::iconChanged, this, [this]{ update(); });
}

void TrayIconPainter::paint(QPainter *painter) {
    if (_trayIcon) {
        auto pixmap = _trayIcon->icon();

        if (!pixmap.isNull() && pixmap.size() != size().toSize())
            pixmap = pixmap.scaled(size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        painter->drawPixmap(QPointF{0, 0}, pixmap);
    }
}
