/**************************************************************************
 *  liveicon.h
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

#ifndef LIVEICON_H
#define LIVEICON_H

#include <qpixmap.h>
#include <qquickpainteditem.h>

class LiveIcon : public QObject {
    Q_OBJECT

public:
    inline LiveIcon() { }

    inline QPixmap icon() { return _icon; }
    void setIcon(QPixmap const& icon);

private:
    QPixmap _icon = {};

signals:
    void iconChanged();
};


class LiveIconPainter : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QObject *liveIcon READ liveIcon WRITE setLiveIconObject NOTIFY liveIconChanged)

public:
    inline LiveIconPainter(QQuickItem *parent = nullptr) :
        QQuickPaintedItem(parent)
    { }

    LiveIcon *liveIcon() { return _liveIcon; }
    void setLiveIconObject(QObject *newLiveIconObject);
    virtual void paint(QPainter *painter) override;

private:
    LiveIcon *_liveIcon = nullptr;

signals:
    void liveIconChanged();
};

#endif // LIVEICON_H
