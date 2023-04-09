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
