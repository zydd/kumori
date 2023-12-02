#ifndef LIVEICON_H
#define LIVEICON_H

#include <qpixmap.h>
#include <qquickpainteditem.h>

class LiveIcon : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY iconChanged)

public:
    inline LiveIcon() { }

    inline QString tooltip() { return _tooltip; }
    inline void setTooltip(QString tooltip) { this->_tooltip = tooltip; }

    inline QPixmap icon() { return _icon; }
    void setIcon(QPixmap const& icon);

private:
    QString _tooltip;
    QPixmap _icon;

signals:
    void iconChanged();
};


class LiveIconPainter : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QObject *liveIcon WRITE setLiveIconObject)

public:
    inline LiveIconPainter(QQuickItem *parent = nullptr) :
        QQuickPaintedItem(parent)
    { }

    void setLiveIconObject(QObject *newLiveIconObject);
    virtual void paint(QPainter *painter) override;

private:
    LiveIcon *_liveIcon;
};

#endif // LIVEICON_H
