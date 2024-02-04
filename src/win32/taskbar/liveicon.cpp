/**************************************************************************
 *  liveicon.cpp
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

#include "liveicon.h"

#include <qpainter.h>


void LiveIcon::setIcon(const QPixmap &icon) {
//    qDebug();

    this->_icon = icon;
    emit iconChanged();
}


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
