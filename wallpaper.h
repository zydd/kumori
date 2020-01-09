#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QQuickPaintedItem>
#include <QImage>

class Wallpaper : public QQuickPaintedItem {
    Q_OBJECT

public:
    Wallpaper();
    void paint(QPainter *painter);

signals:

public slots:

private:
    QImage m_image;
};

#endif // WALLPAPER_H
