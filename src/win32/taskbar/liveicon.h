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
    QPixmap _icon;

signals:
    void iconChanged();
};


class LiveIconPainter : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QObject *liveIcon READ liveIcon WRITE setLiveIconObject)

public:
    inline LiveIconPainter(QQuickItem *parent = nullptr) :
        QQuickPaintedItem(parent)
    { }

    LiveIcon *liveIcon() { return _liveIcon; }
    void setLiveIconObject(QObject *newLiveIconObject);
    virtual void paint(QPainter *painter) override;

private:
    LiveIcon *_liveIcon;
};

#endif // LIVEICON_H
