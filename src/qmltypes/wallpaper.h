/**************************************************************************
 *  wallpaper.h
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

#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <qimage.h>
#include <qquickpainteditem.h>
#include <qtimer.h>

class Wallpaper : public QQuickPaintedItem {
    Q_OBJECT

public:
    Wallpaper();
    void paint(QPainter *painter);

private:
    QImage m_image;
    QTimer m_timer;
    QString m_path;
};

#endif // WALLPAPER_H
