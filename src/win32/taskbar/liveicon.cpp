#include "liveicon.h"

#include <qpainter.h>

void LiveIconPainter::setLiveIconObject(QObject *newLiveIcon) {
    _liveIcon = qobject_cast<LiveIcon *>(newLiveIcon);

    connect(_liveIcon, &LiveIcon::iconChanged, this, [this]{ update(); });
}

void LiveIconPainter::paint(QPainter *painter) {
    if (_liveIcon) {
        auto pixmap = _liveIcon->icon();

        if (!pixmap.isNull() && pixmap.size() != size().toSize())
            pixmap = pixmap.scaled(size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        painter->drawPixmap(QPointF{0, 0}, pixmap);
    }
}

void LiveIcon::setIcon(const QPixmap &icon) {
    this->_icon = icon;
    emit iconChanged();
}
